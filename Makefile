all: client

client: Main.o ColorDetector.o ColorSensor.o Conveyer.o Ev3.o ServerIO.o
	g++ -o client Main.o ColorDetector.o ColorSensor.o Conveyer.o Ev3.o ServerIO.o -lev3dev-c -O3

Main.o: Main.cpp
	g++ -c Main.cpp -O3

ColorDetector.o: ColorDetector.cpp ColorDetector.h
	g++ -c ColorDetector.cpp -O3

ColorSensor.o: ColorSensor.cpp ColorSensor.h
	g++ -c ColorSensor.cpp -lev3dev-c -O3

Conveyer.o: Conveyer.cpp Conveyer.h
	g++ -c Conveyer.cpp -lev3dev-c -O3

Ev3.o: Ev3.cpp Ev3.h
	g++ -c Ev3.cpp -lev3dev-c -O3

ServerIO.o: ServerIO.cpp ServerIO.h
	g++ -c ServerIO.cpp -lev3dev-c -O3

clean:
	rm client Main.o ColorDetector.o ColorSensor.o Conveyer.o Ev3.o ServerIO.o
