. source-me

VERFILE=.version
BUILDNUMFILE=.build

VER=`cat ${VERFILE}`

BLD=`cat ${BUILDNUMFILE}`
BLD=$((${BLD} + 1))
echo $BLD > $BUILDNUMFILE

printf -v VERHEX '%02x' $VER
printf -v BLDHEX '%02x' $BLD

BOARDS="gwc_v2_boot"

mkdir -p bin

for b in $BOARDS; do

	cd fw_main
	#echo $b
	make M=$b clean
	make M=$b bin
	cp FLASH_RUN/$b/$b.bin ../bin/
	cd ..

	arrIN=(${b//_/ })
	if [[ x${arrIN[2]} == x"boot" ]]; then
		../tools/genfw/genfw.py --fw-file ./bin/$b.bin --out-file ./bin/$b-${VERHEX}${BLDHEX}.fw --board ${arrIN[0]}_${arrIN[1]} --crypt-mode CBC --crypt-iv random
		rm ./bin/$b.bin
	fi

done
