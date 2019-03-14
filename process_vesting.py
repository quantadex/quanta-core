import csv
import json
from graphenebase import Account

def qdex(a):
    return int(a * 100000)

with open('./data/genesis_vesting.csv') as csvfile:
    c = csv.DictReader(csvfile, delimiter=',')
    accounts = {}
    for row in c:
        if row["Amount"] == "":
            continue

        shortAddr = str(Account.PublicKey(row["QuantaPubKey"], "QA").address)
        tx = {
                "begin_balance": 0,
                 "amount": qdex(float(row["Amount"].replace(",",""))),
                 "asset_symbol": "QDEX",
                 "begin_timestamp": "2019-03-13T00:00:00",
                 "owner": shortAddr,
                 "vesting_duration_seconds": row["Vesting_Duration_secs"]
             }
        print(json.dumps(tx) + ",")


with open('./data/genesis_vesting.csv') as csvfile:
    c = csv.DictReader(csvfile, delimiter=',')
    accounts = {}
    for row in c:
        if row["Amount"] == "":
            continue

        shortAddr = str(Account.PublicKey(row["QuantaPubKey"], "QA").address)
        tx = {
            "amount": qdex(float(row["Begin Balance"])),
            "asset_symbol": "QDEX",
            "owner": shortAddr
        }

        print(json.dumps(tx) + ",")