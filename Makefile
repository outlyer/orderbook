CPP = g++
CFLAGS = -g -Wall -pipe -Os
DEST = build/Release
TARGET = $(DEST)/Pricer
DIFF = diff
MKDIR = mkdir
RMDIR = rmdir

all:
	$(MKDIR) -p $(DEST)
	$(CPP) $(CFLAGS) -o $(TARGET) Pricer/main.cpp

clean:
	$(RM) $(TARGET)
	$(RM) -rf $(DEST)/*.dSYM
	$(RM) -rf $(DEST)/*.build	
	$(RMDIR) -p $(DEST)


test: all
	$(TARGET) 200 < source/feed.txt > check.tmp
	$(DIFF) check.tmp check.txt # No output means correct results
	$(RM) check.tmp

