# LOG FUNCTIONS

source "utils/colors.sh"

function _print_log() {
    local message
    local prefix
    local color

    [[ $# -ge 3 ]] || exit 1

    color=$1
    prefix=$2
    message=$3

    echo -e "[${color}${prefix}${OFF}]" "${message}"
}

function print_error() {
    _print_log "$(bold 'red')" "error" "$1"
}

function print_warning() {
    _print_log "$(bold 'yellow')" "warning" "$1"
}

function print_info() {
    _print_log "$(bold 'cyan')" "info" "$1"
}
