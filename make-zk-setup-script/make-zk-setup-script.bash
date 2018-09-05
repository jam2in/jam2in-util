#!/bin/bash

source zk_repl.sh
if [ -d ${TARGET_DIR} ]
then
  rm -rf ${TARGET_DIR}
fi
mkdir -p ${TARGET_DIR}

echo "ZK_CLI=\"${ZK_CLI_STR}\"
ZK_ADDR=\"-server ${ZK_ENSEMBLE_ADDR}\"

\$ZK_CLI \$ZK_ADDR create /arcus_repl 0

\$ZK_CLI \$ZK_ADDR create /arcus_repl/client_list 0
\$ZK_CLI \$ZK_ADDR create /arcus_repl/client_list/${SERVICE_CODE} 0

\$ZK_CLI \$ZK_ADDR create /arcus_repl/cache_server_log 0

\$ZK_CLI \$ZK_ADDR create /arcus_repl/cache_list 0
\$ZK_CLI \$ZK_ADDR create /arcus_repl/cache_list/${SERVICE_CODE} 0

\$ZK_CLI \$ZK_ADDR create /arcus_repl/group_list 0
\$ZK_CLI \$ZK_ADDR create /arcus_repl/group_list/${SERVICE_CODE} 0" > ${SETUP_FILE_NAME}

echo "ZK_CLI=\"${ZK_CLI_STR}\"
ZK_ADDR=\"-server ${ZK_ENSEMBLE_ADDR}\"" > ${REMOVE_FILE_NAME}

for((i=0;i<${GROUP_NUM};i++));
do
	eval GNAME=\${GROUP${i}_NAME};
	echo "\$ZK_CLI \$ZK_ADDR create /arcus_repl/group_list/${SERVICE_CODE}/${GNAME} 0" >> ${SETUP_FILE_NAME}
done

echo "
\$ZK_CLI \$ZK_ADDR create /arcus_repl/cache_server_mapping 0" >> ${SETUP_FILE_NAME} 

for((i=0;i<${GROUP_NUM};i++));
do
	eval GNAME=\${GROUP${i}_NAME};
	for((j=0;j<${NODE_NUM};j++));
	do
		eval NODE_ADDR=\${GROUP${i}_NODE${j}_ADDR};
        eval NODE_PORT=\${GROUP${i}_NODE_PORT};
        eval GROUP_PORT=\${GROUP${i}_NODE_GROUP_PORT};
		echo "
\$ZK_CLI \$ZK_ADDR create /arcus_repl/cache_server_mapping/${NODE_ADDR}:${NODE_PORT} 0
\$ZK_CLI \$ZK_ADDR create /arcus_repl/cache_server_mapping/${NODE_ADDR}:${NODE_PORT}/${SERVICE_CODE}^${GNAME}^${NODE_ADDR}:${GROUP_PORT} 0" >> ${SETUP_FILE_NAME}
    echo "
\$ZK_CLI \$ZK_ADDR delete /arcus_repl/cache_server_mapping/${NODE_ADDR}:${NODE_PORT}/${SERVICE_CODE}^${GNAME}^${NODE_ADDR}:${GROUP_PORT}
\$ZK_CLI \$ZK_ADDR delete /arcus_repl/cache_server_mapping/${NODE_ADDR}:${NODE_PORT}" >> ${REMOVE_FILE_NAME}

    eval START_MEMC_FILE_NAME_OUT=${START_MEMC_FILE_NAME}-${GNAME}-${NODE_ADDR}-${NODE_PORT}.bash;
    echo "mkdir -p ${ARCUS_MEMCACHED_DIR}/memc_pid_list

${ARCUS_MEMCACHED_DIR}/memcached -P ${ARCUS_MEMCACHED_DIR}/memc_pid_list/memcached.${NODE_ADDR}:${NODE_PORT} -E ${ARCUS_MEMCACHED_LIB_DIR}/lib/default_engine.so -X ${ARCUS_MEMCACHED_LIB_DIR}/lib/syslog_logger.so -X ${ARCUS_MEMCACHED_LIB_DIR}/lib/ascii_scrub.so -d -v -o 60 -r -R5 -U 0 -D: -b 8192 -m${NODE_MEM} -p ${NODE_PORT} -c ${MAX_CONNS} -t 6 -u ${MEMC_USER_ACCOUNT} -z ${ZK_ENSEMBLE_ADDR}" >> ${START_MEMC_FILE_NAME_OUT}

  chmod +x ${START_MEMC_FILE_NAME_OUT}
	done
done

echo "" >> ${REMOVE_FILE_NAME}

for((i=0;i<${GROUP_NUM};i++));
do
	eval GNAME=\${GROUP${i}_NAME};
  echo "\$ZK_CLI \$ZK_ADDR delete /arcus_repl/group_list/${SERVICE_CODE}/${GNAME}" >> ${REMOVE_FILE_NAME}
done

echo "
\$ZK_CLI \$ZK_ADDR delete /arcus_repl/client_list/${SERVICE_CODE}
\$ZK_CLI \$ZK_ADDR delete /arcus_repl/cache_list/${SERVICE_CODE}
\$ZK_CLI \$ZK_ADDR delete /arcus_repl/group_list/${SERVICE_CODE}" >> ${REMOVE_FILE_NAME}

chmod +x ${SETUP_FILE_NAME}
chmod +x ${REMOVE_FILE_NAME}
