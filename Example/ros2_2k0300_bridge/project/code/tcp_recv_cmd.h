#ifndef TCP_RECV_CMD_H
#define TCP_RECV_CMD_H

#ifdef __cplusplus
extern "C" {
#endif

void tcp_recv_cmd_thd_entry(void);
void close_tcp_recv_cmd(void);

#ifdef __cplusplus
}
#endif

#endif