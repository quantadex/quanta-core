#include <boost/test/unit_test.hpp>

#include <graphene/chain/database.hpp>
#include <graphene/chain/custom_evaluator.hpp>
#include <graphene/chain/hardfork.hpp>

#include <fc/crypto/digest.hpp>
#include <fc/crypto/elliptic.hpp>
#include <fc/reflect/variant.hpp>

#include "../common/database_fixture.hpp"

using namespace graphene::chain;

BOOST_FIXTURE_TEST_SUITE( quanta_tests, database_fixture )

    BOOST_AUTO_TEST_CASE( roll_dice_test )
    {
        try {
            uint32_t skip = database::skip_witness_signature
                            | database::skip_transaction_signatures
                            | database::skip_transaction_dupe_check
                            | database::skip_block_size_check
                            | database::skip_tapos_check
                            | database::skip_authority_check
                            | database::skip_merkle_check;

            ACTORS((joe)(jane));
            transfer(committee_account, joe_id, asset( 10000*GRAPHENE_BLOCKCHAIN_PRECISION ) );


            transfer(committee_account, jane_id, asset( 10000*GRAPHENE_BLOCKCHAIN_PRECISION ) );
            const asset_object& usdcoin = create_user_issued_asset( "USD", jane, 0);
            asset_id_type usd_id = usdcoin.id;

            issue_uia( jane_id,  asset(200000, usd_id));

            generate_blocks(HARDFORK_CORE_QUANTA2_TIME, true, skip);
            enable_fees();

            flat_set< fee_parameters > new_fees;
            roll_dice_operation::fee_parameters_type fee;
            fee.fee = 100;
            new_fees.insert( fee );
            change_fees(new_fees);

            // lose a lot
            auto processed = roll_dice(joe_id, asset(1000*GRAPHENE_BLOCKCHAIN_PRECISION), ">99", flat_set<uint16_t>());
            db.generate_block(db.get_slot_time(1), db.get_scheduled_witness(1), init_account_priv_key, database::skip_transaction_signatures | database::skip_tapos_check);

            for (int i=0; i<30; i++) {
                asset start = db.get_balance(joe_id, asset_id_type(0));
                processed = roll_dice(joe_id, asset(GRAPHENE_BLOCKCHAIN_PRECISION), ">2", flat_set<uint16_t>());
                db.generate_block(db.get_slot_time(1), db.get_scheduled_witness(1), init_account_priv_key, database::skip_transaction_signatures | database::skip_tapos_check);
                asset end = db.get_balance(joe_id, asset_id_type(0));
                ilog("start = ${start} end = ${end} diff = ${diff}", ("start", start)("end",end)("diff", end-start));
            }

            asset start = db.get_balance(jane_id, usd_id);
            printf("***** JANE **** %ld\n", start.amount.value);

//            update_feed_producers( usd_id(db), {jane.id} );
//            price_feed current_feed;
//            current_feed.settlement_price = asset(100, usd_id) / asset(3*GRAPHENE_BLOCKCHAIN_PRECISION);
//            current_feed.maintenance_collateral_ratio = 1750; // need to set this explicitly, testnet has a different default
//            publish_feed( usd_id(db), jane, current_feed );

//            asset_update_operation update_op;
//            update_op.issuer = jane_id;
//            update_op.asset_to_update = usd_id;
////            update_op.new_issuer = jane_id;
//            // new_options should be optional, but isn't...the following line should be unnecessary #580
//            update_op.new_options = usd_id(db).options;
//            update_op.new_options.core_exchange_rate = asset(100, usd_id) / asset(3*GRAPHENE_BLOCKCHAIN_PRECISION);
//            signed_transaction tx;
//            tx.operations.push_back( update_op );
//            for( auto& op : trx.operations ) db.current_fee_schedule().set_fee(op);
//            tx.set_expiration( db.head_block_time() + 10 * db.get_global_properties().parameters.block_interval );
//            PUSH_TX( db, tx, database::skip_authority_check | database::skip_tapos_check | database::skip_transaction_signatures );

            generate_blocks(HARDFORK_CORE_QUANTA3_TIME, true, skip);
            enable_fees();

            for (int i=0; i<15; i++) {
                asset start = db.get_balance(jane_id, asset_id_type(0));
                processed = roll_dice(jane_id, asset(100, usd_id), ">90", flat_set<uint16_t>());
                db.generate_block(db.get_slot_time(1), db.get_scheduled_witness(1), init_account_priv_key, database::skip_transaction_signatures | database::skip_tapos_check);
                asset end = db.get_balance(jane_id, asset_id_type(0));
                ilog("start = ${start} end = ${end} diff = ${diff}", ("start", start)("end",end)("diff", end-start));
            }

            for (int i=0; i<15; i++) {
                asset start = db.get_balance(jane_id, asset_id_type(0));
                processed = roll_dice(jane_id, asset(100, usd_id), ">90", flat_set<uint16_t>());
                db.generate_block(db.get_slot_time(1), db.get_scheduled_witness(1), init_account_priv_key, database::skip_transaction_signatures | database::skip_tapos_check);
                asset end = db.get_balance(jane_id, asset_id_type(0));
                ilog("start = ${start} end = ${end} diff = ${diff}", ("start", start)("end",end)("diff", end-start));
            }

            //rolldice_evaluator::settle(db, 1000, processed.id(), 0, processed.operations[0].get<roll_dice_operation>(), processed.signatures[0]);

        } catch (fc::exception& e) {
            edump((e.to_detail_string()));
            throw;
        }
    }

BOOST_AUTO_TEST_SUITE_END()
