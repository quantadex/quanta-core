import sys
from graphenebase import Account

print (str(Account.PublicKey(sys.argv[1], "QA").address))