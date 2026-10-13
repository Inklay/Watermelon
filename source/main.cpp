#include "Watermelon.h"

int main()
{
	Watermelon server;
	if (server.start()) {
		server.run();
	}

	return 0;
}
