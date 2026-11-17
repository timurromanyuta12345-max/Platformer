#include <xmmintrin.h>
#include <pmmintrin.h>
#include "Game.h"

int main(int argc, char* argv[]) {
	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
	_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

	Game game;

	while (!game.getWindow()->isDone()) {
		game.update();
		game.render();
		game.lateUpdate();
	}
	
	return 0;
}