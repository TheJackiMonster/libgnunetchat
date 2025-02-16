#!/bin/sh
COUNT=$1
shift 1

$(dirname $0)/.setup.sh
PING=$(dirname $0)/../.build_benchmark/tools/messenger_ping

pong() {
  local INDEX=$1
  shift 1

  local PONGS=$((1 + $COUNT - $INDEX))
  local TIMEOUT=$((1 + $PONGS))

  sleep $INDEX
  $PING -P -c $PONGS -t $TIMEOUT $@ > /dev/null
}

for INDEX in $(seq $COUNT); do
  pong $INDEX $@ &
done

sleep 1.25
$PING -c $COUNT -d 1 -t 1 $@ 

wait
