# CONSOLE COLOR CONSTANTS

function _get_color_code() {
    case "$1" in
        "black") echo '30m';;        # black
        "red") echo '31m';;          # red
        "green") echo '32m';;        # green
        "yellow") echo '33m';;       # yellow
        "blue") echo '34m';;         # blue
        "purple") echo '35m';;       # purple
        "cyan") echo '36m';;         # cyan
        "white") echo '37m';;        # white
    esac
}

# regular colors
function color() {
    echo "\033[0;$(_get_color_code $1)"
}

# bold colors
function bold() {
    echo "\033[1;$(_get_color_code $1)"
}

# underlined colors
function underline() {
    echo "\033[4;$(_get_color_code $1)"
}

# DEFAULT COLORS

OFF='\033[0m'       # Text Reset

BLACK="$(color 'black')"
RED="$(color 'red')"
GREEN="$(color 'green')"
YELLOW="$(color 'yellow')"
BLUE="$(color 'blue')"
PURPLE="$(color 'purple')"
CYAN="$(color 'cyan')"
WHITE="$(color 'white')"
