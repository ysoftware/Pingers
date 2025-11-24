all:
	cc -o server.app server.c lib/mongoose-7.19/mongoose.c
	cc -o client.app client.c lib/mongoose-7.19/mongoose.c
