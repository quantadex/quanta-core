import csv
import json
from graphenebase import Account

with open('./data/genesis_worksheet.csv') as csvfile:
    c = csv.DictReader(csvfile, delimiter=',')
    accounts = {}
    for row in c:
        #print(row["QUANTA Addr"], row["AccountID"])
        accounts[row["AccountID"]] = row["QUANTA Addr"]

    for name, key in accounts.items():
        rec = {
            "name": name,
            "owner_key": key,
            "active_key": key,
            "is_lifetime_member": True
        }
        print(json.dumps(rec) + ",")

with open('./data/genesis_worksheet.csv') as csvfile:
    c = csv.DictReader(csvfile, delimiter=',')
    for row in c:
        shortAddr = str(Account.PublicKey(row["QUANTA Addr"], "QA").address)
        tx = {
                 "amount": int(float(row["QDEX Tokens"])*100000),
                 "asset_symbol": "QDEX",
                 "owner": shortAddr
             }

        print(json.dumps(tx) + ",")