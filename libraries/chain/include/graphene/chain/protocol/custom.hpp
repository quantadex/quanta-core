/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once
#include <graphene/chain/protocol/base.hpp>

namespace graphene { namespace chain { 

   /**
    * @brief provides a generic way to add higher level protocols on top of witness consensus
    * @ingroup operations
    *
    * There is no validation for this operation other than that required auths are valid and a fee
    * is paid that is appropriate for the data contained.
    */
   struct custom_operation : public base_operation
   {
      struct fee_parameters_type { 
         uint64_t fee = GRAPHENE_BLOCKCHAIN_PRECISION; 
         uint32_t price_per_kbyte = 10;
      };

      asset                     fee;
      account_id_type           payer;
      flat_set<account_id_type> required_auths;
      uint16_t                  id = 0;
      vector<char>              data;

      account_id_type   fee_payer()const { return payer; }
      void              validate()const;
      share_type        calculate_fee(const fee_parameters_type& k)const;
   };

   struct roll_dice_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee = GRAPHENE_BLOCKCHAIN_PRECISION; };

      account_id_type   account_id;
      asset             fee;

      asset             risk;
      std::string       bet;   // "odd" , "even", "<n", ">n"
      flat_set<uint16_t> numbers;

      extensions_type extensions;

      account_id_type fee_payer()const { return account_id; }
      void            validate()const;
      share_type        calculate_fee(const fee_parameters_type& k)const;

   };

   struct roll_dice_settle_operation : public base_operation {
      struct fee_parameters_type { uint64_t fee = GRAPHENE_BLOCKCHAIN_PRECISION; };
      account_id_type         account_id;
      uint32_t                block_id;
      transaction_id_type     tx;
      uint32_t                op_index;

      asset          risk;
      uint64_t       outcome;
      bool           win;
      asset          payout;

      extensions_type extensions;

      account_id_type fee_payer()const { return account_id; }
      void            validate()const { FC_ASSERT( !"virtual operation" ); }

      /// This is a virtual operation; there is no fee
      share_type      calculate_fee(const fee_parameters_type& k)const { return 0; }
   };

} } // namespace graphene::chain

FC_REFLECT( graphene::chain::custom_operation::fee_parameters_type, (fee)(price_per_kbyte) )
FC_REFLECT( graphene::chain::custom_operation, (fee)(payer)(required_auths)(id)(data) )

FC_REFLECT( graphene::chain::roll_dice_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::roll_dice_operation, (fee)(account_id)(risk)(bet)(numbers)(extensions) )
