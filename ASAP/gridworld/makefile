CXX	=	clang++-3.8
CCFLAGS	=	-Wall -O3 -ffast-math -msse -I../engine -DNDEBUG
EXTRA	=	-std=c++0x
OBJS	=	main.o
TARGET	=	gridworld

$(TARGET):	$(OBJS)
		$(CXX) $(CCFLAGS) $(EXTRA) -o $(TARGET) $(OBJS)

clean:
		rm -f $(OBJS) $(TARGET) *~ core

.cc.o:
		$(CXX) $(CCFLAGS) $(EXTRA) -c $<
