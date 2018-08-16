#!/bin/bash

SSB_DBGEN_SUBMODULE=ssb-dbgen
SSB_DBGEN_GITREPO=https://github.com/valco1994/ssb-dbgen
SSB_DBGEN_BUILDIR=build

# the following sourced file defines the following variables used across several scripts, so that you need to change them only in one place
# A) AHEAD_SCRIPT_BOOTSTRAP
# B)
# C)
# D)
# 1) AHEAD_SCALEFACTOR_MIN
# 2) AHEAD_SCALEFACTOR_MAX
# 3) AHEAD_DB_PATH
source ./common.conf

mkdir -p "${AHEAD_DB_PATH}"

./${AHEAD_SCRIPT_BOOTSTRAP}
pushd ${AHEAD_BUILD_RELEASE_DIR}
make -j $(nproc) ssbm-dbsize_scalar
popd

# For the reproducibility, use the submodule ssb-dbgen to generate the ssb generator and push it to the database directory
if [[ -z "$(ls -A ${SSB_DBGEN_SUBMODULE})" ]]; then
	git submodule init "${SSB_DBGEN_SUBMODULE}"
fi

git submodule update "${SSB_DBGEN_SUBMODULE}"
pushd "${SSB_DBGEN_SUBMODULE}"
[[ ! -d "${SSB_DBGEN_BUILDIR}" ]] && mkdir -p "${SSB_DBGEN_BUILDIR}"
pushd "${SSB_DBGEN_BUILDIR}"
cmake ..
make
ret=$?
if [[ $ret -ne 0 ]]; then
	echo "Error maing ssb-dbgen!:"
	cat make.err
	popd;popd
	exit
fi

cp dbgen ../../${AHEAD_DB_PATH}
echo "*** copied dbgen executable ***"

popd;popd

# Now, the actual script
SCRIPT_DATABASE_GENERATED_FILE=generated

function untar_headers {
	tar -xaf headers.tgz
	sync
}

declare -A table_names
table_names=([c]='customer' [d]='date' [l]='lineorder' [p]='part' [s]='supplier')

if [[ ! -d ${AHEAD_DB_PATH}/headers ]]; then
	untar_headers
else
	for tab in customer date lineorder part supplier; do
		if [[ ! -f "${tab}_header.csv" || ! -f "${tab}AN_header.csv" ]]; then
			untar_headers;break
		fi
	done
fi

pushd ${AHEAD_DB_PATH}

for sf in $(seq ${AHEAD_SCALEFACTOR_MIN} ${AHEAD_SCALEFACTOR_MAX}); do
	echo "Generating data for scale factor ${sf}"
	sync
	echo "  * synced all files (sync)"

	if [[ -f "${AHEAD_SCALEFACTOR_PREFIX}${sf}/${SCRIPT_DATABASE_GENERATED_FILE}" ]]; then
		echo "  * It seems that all files for scale factor ${sf} are already generated"
		continue
	fi

	all_existing=1
	for tab in c d l p s; do
		filename="${table_names[$tab]}.tbl"
		echo -n "  * ${tab}: ${filename} "
		if [[ -f "$filename" || -f "${AHEAD_SCALEFACTOR_PREFIX}${sf}/$filename" ]]; then
			echo "exists"
		else
			all_existing=0
			./dbgen -s ${sf} -T ${tab} -v 1>/dev/null 2>/dev/null
			ret=$?
			if [[ ! $ret -eq 0 ]]; then
				echo "Error"
				popd
				exit $ret
			fi
			sync
			echo "created"
		fi
	done

	if [[ ${all_existing} == 0 ]]; then
		sync
		echo "  * synced all files (sync)"
		mkdir -p "${AHEAD_SCALEFACTOR_PREFIX}${sf}"
		ret=$?
		if [[ $ret -ne 0 ]]; then
			popd
			echo "  * Error creating folder '${AHEAD_SCALEFACTOR_PREFIX}${sf}'"
			exit $ret
		fi
#		ls -lAh *.tbl
		mv *.tbl "${AHEAD_SCALEFACTOR_PREFIX}${sf}/"
		ret=$?
		if [[ $ret -ne 0 ]]; then
			popd
			echo "  * Error moving files to subfolder '${AHEAD_SCALEFACTOR_PREFIX}${sf}'"
			exit $ret
		fi
		echo "  * generated and moved tables for SF ${sf}"
	else
		echo "  * all tables already exist for SF ${sf}"
	fi

	pushd "${AHEAD_SCALEFACTOR_PREFIX}${sf}"

	for tab in customer date lineorder part supplier; do
		if [[ ! -f "${tab}.tbl" ]]; then
			popd;popd
			echo "  * Error: data file '${tab}.tbl' does not exist"
			exit 1
		fi
		if [[ ! -f ${tab}_header.csv ]]; then
			ln -s ../headers/${tab}_header.csv ${tab}_header.csv
			ret=$?
			if [[ $ret -ne 0 ]]; then
				popd;popd
				echo "  * Error creating softlink ${tab}_header.csv -> ../headers/${tab}_header.csv"
				exit $ret
			fi
		else
			echo "  * ${tab}_header.csv exists"
		fi
		if [[ ! -f ${tab}AN_header.csv ]]; then
			ln -s ../headers/${tab}AN_header.csv ${tab}AN_header.csv
			ret=$?
			if [[ $ret -ne 0 ]]; then
				echo "  * Error creating softlink ${tab}AN_header.csv -> ../headers/${tab}AN_header.csv"
				exit $ret
			fi
			#sed -e '/TINYINT/RESTINY/G' -e '/SHORTINT/RESSHORT/g' -e '/INTEGER/RESINT/g' -i ${tab}AN_header.csv
		else
			echo "  * ${tab}AN_header.csv exists"
		fi
		if [[ ! -f ${tab}AN.tbl ]]; then
			ln -s ${tab}.tbl ${tab}AN.tbl
			ret=$?
			if [[ $ret -ne 0 ]]; then
				echo "  * Error creating softlink ${tab}AN.tbl -> ${tab}.tbl"
				exit $ret
			fi
		else
			echo "  * ${tab}AN.tbl exists"
		fi
	done

	echo "========================================================"
	echo "=== GENERATING SMALLER DATABASE FILES. DO NOT ABORT! ==="
	echo "========================================================"
	../../build/Release/ssbm-dbsize_scalar -d . 
	ret=$?
	if [[ $ret -ne 0 ]]; then
		echo "  * Error executing ssbm-dbsize_scalar"
		exit $ret
	fi

	touch "${SCRIPT_DATABASE_GENERATED_FILE}"
	sync

	rm -f *.tbl
	ret=$?
	if [[ $ret -ne 0 ]]; then
		echo "  * Error removing table files"
		exit $ret
	fi

	popd

	/bin/bash -c "ls *.tbl" &>/dev/null
	ret2=$?
	if [[ $ret2 -eq 0 ]]; then
		echo "  * There are still some table files here!"
		rm -f *.tbl
	fi

	sync
	echo "  * synced all files (sync)"

done

popd
