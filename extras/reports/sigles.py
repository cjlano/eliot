#!/usr/bin/env python

import os
import sys
import argparse

# Eliot modules
import eliot


# Command-line parsing
parser = argparse.ArgumentParser(description="""Generate a game summary suitable for the SIGLES program
(used by the French Scrabble Federation for official tournaments)""")
parser.add_argument("-s", "--savegame", help="game saved with Eliot", type=file, required=True)
parser.add_argument("-o", "--output", help="output file in latin1 encoding (stdout by default)",
        type=eliot.FileType(mode='w', encoding="latin1"), default=sys.stdout)
parser.add_argument("-n", "--no-empty", help="do not output lines for empty tables", action="store_true")
args = parser.parse_args()

# Do most of the work: open the save game file, parse it,
# and build easy to use data structures
gameData = eliot.readSaveGame(args.savegame)

# Output file
out = args.output

# Index the players by their table number
playerByTable = dict([(p.tableNb, p) for p in gameData.players])
# Generate the report
for num in range(1, 1 + max(playerByTable.keys())):
    if num in playerByTable:
        p = playerByTable[num]
        out.write('"%s","%s","%s"\n' % (num, p.name, p.totalScore))
    elif not args.no_empty:
        out.write('"%s","%s","%s"\n' % (num, "--table vide--", -1))

