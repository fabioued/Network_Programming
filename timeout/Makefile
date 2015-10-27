all: receiver_1 receiver_2 receiver_3 sender_1 sender_2 sender_3

receiver_1: alarm/recv_timeo.c
		gcc alarm/recv_timeo.c -o alarm/Recv_timeo
receiver_2: select/recv_timeo.c
		gcc select/recv_timeo.c -o select/Recv_timeo
receiver_3: setsockopt/recv_timeo.c
		gcc setsockopt/recv_timeo.c -o setsockopt/Recv_timeo
sender_1: alarm/send_timeo.c
		gcc alarm/send_timeo.c -o alarm/Send_timeo
sender_2: select/send_timeo.c
		gcc select/send_timeo.c -o select/Send_timeo
sender_3: setsockopt/send_timeo.c
		gcc setsockopt/send_timeo.c -o setsockopt/Send_timeo
