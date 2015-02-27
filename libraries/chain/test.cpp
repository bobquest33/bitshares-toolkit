#include <bts/chain/database.hpp>
#include <bts/chain/asset_operations.hpp>
#include <bts/chain/account_index.hpp>
#include <bts/chain/key_object.hpp>

#include <fc/crypto/digest.hpp>

#define BOOST_TEST_MODULE ChainDatabaseTests
#include <boost/test/included/unit_test.hpp>

using namespace bts::chain;

struct database_fixture {
   database db;
   signed_transaction trx;
   key_id_type genesis_key;
   fc::ecc::private_key private_key = fc::ecc::private_key::generate();

   database_fixture()
   {
      db.init_genesis();
      BOOST_REQUIRE(key_id_type()(db));
      genesis_key = key_id_type()(db)->id;
      db.push_undo_state();
   }
   ~database_fixture(){}

   create_account_operation make_account() {
      create_account_operation create_account;
      create_account.name = "nathan";

      create_account.paying_account = db.get_account_index().get("init0")->id;
      create_account.owner.add_authority(genesis_key, 123);
      create_account.active.add_authority(genesis_key, 321);
      create_account.memo_key = genesis_key;
      create_account.voting_key = genesis_key;
      create_account.registration_fee = asset();

      return create_account;
   }
};

BOOST_FIXTURE_TEST_SUITE( operation_unit_tests, database_fixture )

BOOST_AUTO_TEST_CASE( fail_create_account )
{
   trx.operations.push_back(make_account());
   //Transaction is not signed; should fail.
   BOOST_CHECK_THROW(db.push_transaction(trx), fc::exception);
}

BOOST_AUTO_TEST_CASE( create_account )
{
   trx.operations.push_back(make_account());
   trx.signatures.push_back(fc::ecc::private_key::regenerate(fc::sha256::hash(string("genesis"))).sign_compact(fc::digest(trx)));
   db.push_transaction(trx);

   const account_object* nathan_account = db.get_account_index().get("nathan");
   BOOST_REQUIRE(nathan_account);
   BOOST_CHECK(nathan_account->id.space() == protocol_ids);
   BOOST_CHECK(nathan_account->id.type() == account_object_type);
   BOOST_CHECK(nathan_account->name == "nathan");
   BOOST_CHECK(nathan_account->authorized_assets.empty());
   BOOST_CHECK(nathan_account->delegate_votes.empty());

   BOOST_REQUIRE(nathan_account->owner.auths.size() == 1);
   BOOST_CHECK(nathan_account->owner.auths.at(genesis_key) == 123);
   BOOST_REQUIRE(nathan_account->active.auths.size() == 1);
   BOOST_CHECK(nathan_account->active.auths.at(genesis_key) == 321);
   BOOST_CHECK(nathan_account->voting_key == genesis_key);
   BOOST_CHECK(nathan_account->memo_key == genesis_key);

   const account_balance_object* balances = nathan_account->balances(db);
   BOOST_REQUIRE(balances);
   BOOST_CHECK(balances->id.space() == implementation_ids);
   BOOST_CHECK(balances->id.type() == impl_account_balance_object_type);
   BOOST_CHECK(balances->balances.empty());

   const account_debt_object* debts = nathan_account->debts(db);
   BOOST_REQUIRE(debts);
   BOOST_CHECK(debts->id.space() == implementation_ids);
   BOOST_CHECK(debts->id.type() == impl_account_debt_object_type);
   BOOST_CHECK(debts->call_orders.empty());
}

BOOST_AUTO_TEST_SUITE_END()
