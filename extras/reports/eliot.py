import xml.etree.ElementTree as ET
import codecs


# Define some classes to store parsed data in a structured way

class Move(object):
    """Data for a move. The 'type' and 'points' members are always valid.
    Other members depend on the type of move:
        - for valid and invalid moves, word and coords can be used
        - for a move changing letters, word can be used (it contains the changed letters)
        - otherwise, no other field is valid"""

    def __init__(self, type, points, word, coords):
        self.type = type
        self.points = int(points)
        self.word = word
        self.coords = coords

    def __str__(self):
        return "Move (type=%s): %s" % (self.type, self.points)

    def isValid(self):
        return self.type == "valid"

    def isInvalid(self):
        return self.type == "invalid"

    def isChange(self):
        return self.type == "change"

    def isPass(self):
        return self.type == "pass"

    def isNone(self):
        return self.type == "none"

class Turn(object):
    """Data for a player turn (or game turn)"""

    def __init__(self, num, rack, move, events):
        self.num = num
        self.rack = rack
        self.move = move
        self.events = events

class Player(object):
    """Statistics of a player"""

    def __init__(self, id, name):
        self.id = id
        self.name = name
        self.tableNb = None
        self.turns = {}

class GameData(object):
    """Entry point for all the game data"""

    def __init__(self, name):
        self.players = []
        self.turns = {}
        self.turnNb = 0
        self.name = name
        self.mode = None

    def isTop(self, turnNb, move):
        if move is None:
            return False
        movePoints = move.points
        masterPoints = self.turns[turnNb].move.points
        return masterPoints <= movePoints

    def isSubTop(self, turnNb, move):
        if move is None:
            return False
        playerPoints = [p.turns[turnNb].move.points for p in self.players]
        return move.points >= max(playerPoints)

    def isSolo(self, turnNb, move):
        if move is None:
            return False
        playerPoints = [p.turns[turnNb].move.points for p in self.players]
        return self.isSubTop(turnNb, move) and playerPoints.count(move.points) == 1

def readSaveGame(xmlFileName):

    # Entry point for all the data
    gameData = GameData(xmlFileName)

    # XML tree
    tree = ET.parse(xmlFileName)

    gameData.mode = tree.findtext("Game/Mode")

    # Parse players
    for playerElem in tree.findall("Game/Player"):
        player = Player(
                int(playerElem.get("id")),
                playerElem.findtext("Name"))
        # Priori to Eliot 2.1, there was no table number in the file
        tableNb = playerElem.findtext("TableNb");
        if tableNb is None:
            player.tableNb = player.id
        else:
            player.tableNb = int(tableNb)

        gameData.players.append(player)

    # Parse turns
    turnNum = 0
    for turnElem in tree.findall("History/Turn"):
        turnNum += 1

        # Get players data for this turn
        for p in gameData.players:
            # Get the player rack
            allRacks = turnElem.findall("PlayerRack[@playerId='%s']" % p.id)
            # Before Eliot 2.1 (version 2.0 only), the attribute was called "playerid"
            allRacks.extend(turnElem.findall("PlayerRack[@playerid='%s']" % p.id))
            if len(allRacks) == 0 and turnNum > 1:
                rack = p.turns[turnNum - 1].rack
            else:
                # FIXME: does not work well in freegame mode,
                # because we take the next rack instead of the current one
                rack = allRacks[-1].text

            # Get the player move
            allMoves = turnElem.findall("PlayerMove[@playerId='%s']" % p.id)
            # Before Eliot 2.1 (version 2.0 only), the attribute was called "playerid"
            allMoves.extend(turnElem.findall("PlayerMove[@playerid='%s']" % p.id))
            if len(allMoves) == 0:
                move = Move("none", "0", "", "")
            else:
                lastMove = allMoves[-1]
                move = Move(lastMove.get("type"),
                        lastMove.get("points"),
                        lastMove.get("word"),
                        lastMove.get("coord"))
                if move.isChange():
                    move.word = lastMove.get("letters")
            # Create turn
            # TODO: handle events
            turn = Turn(turnNum, rack, move, None)
            p.turns[turnNum] = turn

        # Get game data for this turn
        rack = turnElem.findtext("GameRack")
        gameMove = turnElem.find("GameMove")
        if gameMove is None:
            move = Move("none", "0", "", "")
        else:
            move = Move(gameMove.get("type"),
                    gameMove.get("points"),
                    gameMove.get("word"),
                    gameMove.get("coord"))
            if move.isChange():
                move.word = lastMove.get("letters")
        gameData.turns[turnNum] = Turn(turnNum, rack, move, None);

    gameData.turnNb = turnNum

    # Parse statistics (introduced in version 2.1: version 2.0 didn't have that)
    if tree.find("Statistics/GameStats") is None:
        # For Eliot 2.0 only (version 2.1 introduced the tag)
        gameData.totalScore = sum([t.move.points for t in gameData.turns.values()])

        for p in gameData.players:
            p.rawScore = sum([t.move.points for t in p.turns.values()])
            # FIXME: should be computed from events
            p.warningsNb = 0
            p.penaltiesPoints = 0
            p.solosPoints = 0
            p.totalScore = p.rawScore + p.warningsNb + p.penaltiesPoints + p.solosPoints
            p.diffWithTop = gameData.totalScore - p.totalScore
            # TODO: percent
            p.percentTop = ""
            # TODO: rank
            p.rank = 0
    else:
        playersById = dict([(p.id, p) for p in gameData.players])
        for playerStat in tree.findall("Statistics/PlayerStats"):
            playerId = int(playerStat.get("playerId"))
            player = playersById[playerId]
            assert player != None, \
                    "No player found with id %s" % playerId

            player.rawScore = int(playerStat.get("rawScore"))
            player.warningsNb = int(playerStat.get("warningsNb"))
            player.penaltiesPoints = int(playerStat.get("penaltiesPoints"))
            player.solosPoints = int(playerStat.get("solosPoints"))
            player.totalScore = int(playerStat.get("totalScore"))
            player.diffWithTop = int(playerStat.get("diffWithTop"))
            player.percentTop = playerStat.get("percentTop")
            player.rank = int(playerStat.get("rank"))
        gameData.totalScore = tree.find("Statistics/GameStats").get("totalScore")

    return gameData


# Utility class, to replace argparse.FileType until Issue 11175 is solved (http://bugs.python.org/issue11175)
class FileType(object):
    """Factory for creating file object types

    Instances of FileType are typically passed as type= arguments to the
    ArgumentParser add_argument() method.

    Keyword Arguments:
       - mode -- A string indicating how the file is to be opened. Accepts the
           same values as the builtin open() function.
       - bufsize -- The file's desired buffer size. Accepts the same values as
           the builtin open() function.
       - encoding -- The file's encoding. Accepts the same values as the
           the builtin open() function.
       - errors -- A string indicating how encoding and decoding errors are to
           be handled. Accepts the same value as the builtin open() function.
    """

    def __init__(self, mode='r', bufsize=-1, encoding=None, errors=None):
        self._mode = mode
        self._bufsize = bufsize
        self._encoding = encoding
        self._errors = errors

    def __call__(self, string):
        # the special argument "-" means sys.std{in,out}
        if string == '-':
            if 'r' in self._mode:
                return _sys.stdin
            elif 'w' in self._mode:
                return _sys.stdout
            else:
                msg = _('argument "-" with mode %r') % self._mode
                raise ValueError(msg)

        # all other arguments are used as file names
        try:
            return codecs.open(string, self._mode, self._encoding, self._errors, self._bufsize)
        except IOError as e:
            message = _("can't open '%s': %s")
            raise ArgumentTypeError(message % (string, e))

    def __repr__(self):
        args = self._mode, self._bufsize
        kwargs = [('encoding', self._encoding), ('errors', self._errors)]
        args_str = ', '.join([repr(arg) for arg in args if arg != -1] +
                             ['%s=%r' % (kw, arg) for kw, arg in kwargs
                              if arg is not None])
        return '%s(%s)' % (type(self).__name__, args_str)

