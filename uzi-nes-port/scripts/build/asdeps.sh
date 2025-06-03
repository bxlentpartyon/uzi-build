#! /bin/bash
ASFILE="${1}"
OFILE=$(echo "${ASFILE}" | sed 's#\.S#\.oS#')
DFILE=${ASFILE}.d

SED_PATTERN='s#.*\"\(.*\)\".*#\1#'
header_list=$(grep '\.include' ${ASFILE} | sed ${SED_PATTERN})
bin_list=$(grep '\.incbin' ${ASFILE} | sed ${SED_PATTERN})

expanded_header_list=""
for header in ${header_list}; do
	for incflag in ${CA_INCLUDE}; do
		incpath=$(echo "${incflag}" | sed 's/-I//')
		f="${incpath}/${header}"
		if [ -f "${f}" ]; then
			expanded_header_list="${expanded_header_list} ${f}"
			break
		fi
	done
done

expanded_bin_list=""
for bin in ${bin_list}; do
	binpath=${PWD}/${bin}
	if [ -f "${binpath}" ]; then
		expanded_bin_list="${expanded_bin_list} ${binpath}"
	fi
done

cat << EOF > ${DFILE}
${OFILE}: ${ASFILE} ${expanded_header_list} ${expanded_bin_list}

${ASFILE} ${expanded_header_list}:
EOF
