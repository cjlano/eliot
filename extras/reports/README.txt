This directory contains several Python scripts to generate reports from Eliot
save games (in XML format). To run them, you need python installed, and in the
case of stats.py, you also need the Cheetah template engine
(http://www.cheetahtemplate.org/, package python-cheetah on Debian systems).

At the moment, the following scripts are supported:
    - stats.py generates an HTML page with information roughly equivalent to
      the Statistics window in Eliot
    - sigles.py generates a game summary, usable with the SIGLES program (used
      in  French Scrabble tournaments)

Each script gives usage information when called with a -h or --help argument.

Note that eliot.py is not directly usable: it contains utility functions
(mainly to extract data from the save game) that are called from the other
scripts.

If you write a new report that you find useful, please consider contributing
it!

