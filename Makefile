CC     = gcc
TARGET = simulador.exe
SRCS   = main.c memory.c pipeline.c stages.c hazard.c io.c

$(TARGET): $(SRCS)
	$(CC) $(SRCS) -o $(TARGET)

clean:
	-del $(TARGET) 2>NUL
	-del pipeline_trace.json 2>NUL
