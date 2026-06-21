
SRC = src/main.cpp $(wildcard src/graphics/*.cpp)

INC = -Isrc -Iinclude -Iinclude/graphics -I"C:/raylib/raylib/src"

EMFLAGS = -o index.html -L"C:/raylib/raylib/src" -lraylib -DPLATFORM_WEB -sUSE_GLFW=3 -sASYNCIFY --preload-file assets@/assets --shell-file "C:/raylib/raylib/src/shell.html"

all: build inject run

build:
	@echo === Compiling Project with Emscripten ===
	emcc $(SRC) $(INC) $(EMFLAGS)

inject:
	@echo === Injecting Fullscreen and Resize Patches ===
	powershell -Command "Add-Content index.html '<style>#header { display: none !important; } body, html { margin: 0; padding: 0; overflow: hidden; background: #000; } canvas { width: 100vw !important; height: 100vh !important; display: block; }</style><script>function fixSize(){var c=document.getElementById(\"canvas\");if(c){c.width=window.innerWidth;c.height=window.innerHeight;if(typeof GL!==\"undefined\"&&GL.currentContext){GL.currentContext.gl.viewport(0,0,c.width,c.height);}}}window.addEventListener(\"resize\",fixSize);setInterval(fixSize,200);</script>'"

run:
	@echo === Launching Local Server via emrun ===
	emrun index.html