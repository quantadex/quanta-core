//
// Created by quoc on 3/10/19.
//
#include <iostream>

#include <fc/smart_ref_impl.hpp>
#include <fc/crypto/hmac.hpp>
#include <fc/crypto/hex.hpp>

#include <graphene/chain/custom_evaluator.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/is_authorized_asset.hpp>
#include <graphene/chain/hardfork.hpp>

bool is_number_(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(),
                                      s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}
namespace graphene { namespace chain {
   void_result rolldice_evaluator::do_evaluate( const roll_dice_operation& op ){
      try {

         const database& d = db();

         const account_object& from_account    = op.account_id(d);
         const asset_object&   asset_type      = op.risk.asset_id(d);

         try {

            GRAPHENE_ASSERT(
                    is_authorized_asset( d, from_account, asset_type ),
                    transfer_from_account_not_whitelisted,
                    "'from' account ${from} is not whitelisted for asset ${asset}",
                    ("from",op.account_id)
                            ("asset",op.risk.asset_id)
            );

             bool is_range_bet = false;
             if (op.bet.length() > 0) {
                 if (op.bet[0] == '>' || op.bet[0] == '<') {
                     if(is_number_(op.bet.substr(1))) {
                         is_range_bet = true;
                         const int num = atoi(op.bet.substr(1).c_str());
                         if (d.head_block_time() >= HARDFORK_CORE_QUANTA3_TIME) {
                             FC_ASSERT( num > 1 && num < 100);
                         }
                     }
                 }
             }

            bool insufficient_balance = d.get_balance( from_account, asset_type ).amount >= op.risk.amount;

            FC_ASSERT( insufficient_balance,
                       "Insufficient Balance: ${balance}, unable to bet '${bet_amount}' from account '${a}'",
                      ("bet_amount",d.to_pretty_string(op.risk))("balance",d.to_pretty_string(d.get_balance(from_account, asset_type)))("a",from_account.name) );

            return void_result();
         } FC_RETHROW_EXCEPTIONS( error, "Unauthorized account ${a} from ${f}", ("a",d.to_pretty_string(op.risk.amount))("f",op.account_id(d).name) );

      }  FC_CAPTURE_AND_RETHROW( (op) ) }

   /* generate_random deterministic by getting the header_hash
    *
    */
   uint64_t generate_random(string c, string data, uint64_t min, uint64_t max) {
      fc::hmac_sha512 mac;
      fc::sha512 l = mac.digest( c.c_str(), c.size(), data.c_str(), data.size() );
      fc::bigint n = fc::bigint(l.data(), l.data_size());
      return (fc::bigint(min) + (n % fc::bigint(max-min))).to_int64();
   }

    uint64_t reject_sampling(std::string c, std::string data, uint64_t min, uint64_t max) {
        uint64_t usable = UINT32_MAX / (max-min);
        const char *current_data = data.c_str();
        size_t current_data_size = data.size();
        uint64_t retrieved = 0;

        do {
            fc::hmac_sha256 mac;
            fc::sha256 h = mac.digest( c.c_str(), c.size(), current_data, current_data_size );
            current_data = h.data();
            current_data_size = h.data_size();
            retrieved = generate_random(c, h, 0, UINT32_MAX);
        } while(retrieved < usable);

        return (retrieved % max) + min;
    }

   void_result rolldice_evaluator::do_apply( const roll_dice_operation& op ){
      database& d = db();
      const account_object& from_account    = op.account_id(d);
      const account_statistics_object &stats = from_account.statistics(d);

      d.modify(stats, [](account_statistics_object& s) {
         if (s.extensions.value.number_of_bets.valid()) {
            *s.extensions.value.number_of_bets = *s.extensions.value.number_of_bets + 1;
         } else {
            s.extensions.value.number_of_bets = 1;
         }
      });

      return void_result();
   }

   void rolldice_evaluator::settle( database &d, uint32_t block_id, transaction_id_type tx, uint32_t op_index, const roll_dice_operation& op, signature_type blocksig)
   {
      const account_object& from_account    = op.account_id(d);
      const asset_object&   asset_type      = op.risk.asset_id(d);
       bool after_fork3 = d.head_block_time() >= HARDFORK_CORE_QUANTA3_TIME;

      std::ostringstream out;
      out << fc::to_hex((const char*)blocksig.begin(), blocksig.size()) << "," << fc::to_hex((const char*)tx.data(), tx.data_size()) << "," << op_index;

      uint64_t randomN;

      if (after_fork3) {
          randomN = reject_sampling(d.get_chain_id().str(), out.str(), 1, 100);
      } else {
          randomN = generate_random(d.get_chain_id().str(), out.str(), 1, 100);
      }

      bool win = false;
      double reward = 0.0;

      if (op.numbers.size() == 0) {
         if (op.bet == "odd") {
            reward = 0.5;
            if (randomN % 2 == 1) {
               win = true;
            }
         } else if(op.bet == "even"){
            if (randomN % 2 == 0) {
               win = true;
            }
         }

         if (op.bet[0] == '>' || op.bet[0] == '<') {
            int betNum = atoi(op.bet.substr(1).c_str());
            if (op.bet[0] == '>') {
               if (after_fork3) 
		 win = randomN > betNum;
               else 
	         win = randomN >= betNum;
               reward = 1./((100.-betNum)/100.);
            }
            if (op.bet[0] == '<') {
               if (after_fork3) {
                   win = randomN < betNum;
                   reward = 1./((betNum-1)/100.);
               } else {
                   win = randomN <= betNum;
                   reward = 1./((betNum)/100.);
               }
            }
         }
      } else {
         flat_set<uint16_t>::const_iterator result = op.numbers.find(randomN);
         if (result != op.numbers.end()) {
            win = true;
            reward = 1./(op.numbers.size()/100.);
         }
      }

      uint16_t fee = d.get_global_properties().parameters.extensions.value.roll_dice_percent_of_fee.valid() ?
              *d.get_global_properties().parameters.extensions.value.roll_dice_percent_of_fee : GRAPHENE_1_PERCENT;

      float fee_dec = float(fee) / GRAPHENE_100_PERCENT;
      bool after_fork = d.head_block_time() >= HARDFORK_CORE_QUANTA2_TIME;

      ilog("roll_dice outcome=${o} ${bet} ${data} win=${win} fee=${fee} reward=${reward}", ("bet", op.bet.c_str())("o",randomN)("data", out.str().c_str())("win",win)("fee",fee)("reward", reward));

      asset payout_obj;
      asset fee_pool;
      asset qdex_amount;

      if (win) {
         float payrate = reward - 1;
         fc::safe<int64_t> payout = op.risk.amount * payrate;

         if (after_fork) {
             uint32_t payrateI = payrate * GRAPHENE_100_PERCENT;
             fc::uint128 r(op.risk.amount.value);
             r *= payrateI;
             r /= GRAPHENE_100_PERCENT;
             payout = r.to_uint64();
         }

         asset balance = d.get_balance(account_id_type(2), op.risk.asset_id);

         share_type available_payout = std::min(payout, balance.amount);
         asset full_payout = asset(available_payout, op.risk.asset_id);

         share_type fee_to_pool = fee_dec * (payout + op.risk.amount);
         share_type payout_amount;

          if (after_fork) {
             fee_to_pool = cut_fee(payout + op.risk.amount, fee);

             // we don't even have enough to pay fee pool
             // just pay remaining to user
             if (after_fork3) {
                 if (fee_to_pool > available_payout) {
                     fee_to_pool = cut_fee(available_payout, fee); // pay available
                 }
                 payout_amount = available_payout - fee_to_pool;
                 full_payout.amount = payout_amount; // keep in the account
                 fee_to_pool = 0;
             } else {
                 if (fee_to_pool > available_payout) {
                     fee_to_pool = 0;
                     payout_amount = available_payout;
                 } else {
                     payout_amount = available_payout - fee_to_pool;
                 }
             }

         } else {
              // we don't even have enough to pay fee pool
              // just pay remaining to user
              if (fee_to_pool > available_payout) {
                  fee_to_pool = 0;
                  payout_amount = available_payout;
              } else {
                  payout_amount = available_payout - fee_to_pool;
              }
          }

         payout_obj = asset(payout_amount,op.risk.asset_id);
         fee_pool = asset(fee_to_pool,op.risk.asset_id);

         d.adjust_balance(account_id_type(2), -full_payout);
         d.adjust_balance(op.account_id, payout_obj);

         ilog("win! risk= ${risk} payout = ${payout} fee_pool = ${fee_pool} qdex_fee = ${qdex}",("risk", op.risk.amount)("payout", payout_obj.amount)("fee_pool",fee_pool.amount)("qdex", qdex_amount.amount));

         // pay fee pool if we can.
         if (fee_pool.amount > share_type(0)) {
             const auto &recv_dyn_data = asset_type.dynamic_asset_data_id(d);
             d.modify(recv_dyn_data, [&](asset_dynamic_data_object &obj) {
                 obj.accumulated_fees += fee_to_pool;
             });
         }

      } else {
          if (after_fork3) {
              d.adjust_balance(op.account_id, -op.risk);
              d.adjust_balance(account_id_type(2), op.risk);
              payout_obj = -op.risk;
          } else {
              share_type fee_to_pool = op.risk.amount * fee_dec;
              if (after_fork) {
                  fee_to_pool = cut_fee(op.risk.amount,fee);
              }
              asset pot_amount = asset(op.risk.amount-fee_to_pool, op.risk.asset_id);


              fee_pool = asset(fee_to_pool,op.risk.asset_id);

              d.adjust_balance(op.account_id, -op.risk);
              d.adjust_balance(account_id_type(2), pot_amount);
              payout_obj = -op.risk;

              const auto &recv_dyn_data = asset_type.dynamic_asset_data_id(d);
              d.modify(recv_dyn_data, [&](asset_dynamic_data_object &obj) {
                  obj.accumulated_fees += fee_to_pool;
              });
          }
      }

      roll_dice_settle_operation op_settle;
      op_settle.account_id = op.account_id;
      op_settle.block_id = block_id;
      op_settle.tx = tx;
      op_settle.qdex_fee = qdex_amount;
      op_settle.op_index = op_index;
      op_settle.risk = op.risk;
      op_settle.bet = op.bet;
      op_settle.outcome = randomN;
      op_settle.win = win;
      op_settle.fee_pool = fee_pool;
      op_settle.payout = payout_obj;
      d.push_applied_operation(op_settle);

   }
}}
