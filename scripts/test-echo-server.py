import socket

def udp_echo_server(host, port):
	# Create a UDP socket
	with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as server_socket:
		# Bind to the specified host and port
		server_socket.bind((host, port))
		print("UDP echo server is listening on {}:{}".format(host, port))

		# Receive and echo messages indefinitely
		while True:
			# Receive data from the client
			data, address = server_socket.recvfrom(1024)
			print("Received message from {}".format(address))

			# Echo the received data back to the client
			server_socket.sendto(data, address)
			print("Echoed message to {}".format(address))

if __name__ == "__main__":
	host = "10.0.0.1"
	port = 8080
	udp_echo_server(host, port)
