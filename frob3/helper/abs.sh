for x
do
  case "$x" in
    /.* ) echo "$x" ;;
    * ) echo "$(/bin/pwd)/$x" ;;
  esac
done
