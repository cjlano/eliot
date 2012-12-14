#!/usr/bin/env python

import os
import sys
import argparse
from Cheetah.Template import Template

# Eliot modules
import eliot

tmpl_file = "stats.tmpl"
css_file = "stats.css"

# Command-line parsing
parser = argparse.ArgumentParser(description="""Generate an HTML page roughly equivalent to the Statistics window in Eliot""")
parser.add_argument("-s", "--savegame", help="game saved with Eliot", type=file, required=True)
parser.add_argument("-o", "--output", help="output file (stdout by default)",
        type=argparse.FileType('w'), default=sys.stdout)
parser.add_argument("-e", "--embed-css", help="embed the CSS file into the resulting HTML file", default=False, action="store_true")
args = parser.parse_args()

# Do most of the work: open the save game file, parse it,
# and build easy to use data structures
gameData = eliot.readSaveGame(args.savegame.name)

# Load the template
with file(tmpl_file) as f:
    templDef = f.read()

# Fill it with values
templ = Template(templDef,
        {'gameData': gameData,
            'embedCss': args.embed_css,
            'cssContents': file(css_file).read()})

# Print the generated document
args.output.write(str(templ));

