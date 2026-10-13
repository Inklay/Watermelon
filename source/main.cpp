#include "Watermelon.h"

int main()
{
	Watermelon server("0.0.0.0", 25565);
	if (server.start()) {
		server.run();
	}

	return 0;
}
