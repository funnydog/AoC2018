import std.array;
import std.format;
import std.stdio;

struct Marble {
	size_t value;
	Marble* prev;
	Marble* next;

	this(size_t value) {
		this.value = value;
		this.prev = &this;
		this.next = &this;
	}

	Marble* insertAfter(Marble *m) {
		m.next = this.next;
		m.prev = &this;
		this.next.prev = m;
		this.next = m;
		return m;
	}

	Marble* removeAndNext() {
		this.prev.next = this.next;
		this.next.prev = this.prev;
		return this.next;
	}
}

int main(string[] args) {
	if (args.length < 2) {
		stderr.writef("Usage: %s <filename>\n", args[0]);
		return -1;
	}

	File input;
	try {
		input = File(args[1], "r");
	} catch (std.exception.ErrnoException) {
		stderr.writef("Cannot open %s for reading\n", args[1]);
		return -1;
	}

	size_t numplayers, lastmarble;
	try {
		input.readf("%d players; last marble is worth %d points",
			    &numplayers, &lastmarble);
	} catch (FormatException) {
		stderr.writef("Cannot read the numplayers and lastmarble\n");
		return -1;
	} finally {
		input.close();
	}


	ulong[] players = new ulong[numplayers];
	Marble* marbles = new Marble(0);
	Marble* curmarble = marbles;

	size_t curplayer = 0;
	for (ulong i = 1; i < lastmarble+1; ++i) {
		if (i % 23 != 0) {
			curmarble = curmarble.next.insertAfter(new Marble(i));
		} else {
			players[curplayer] += i;
			for (int j = 0; j < 7; j++) {
				curmarble = curmarble.prev;
			}
			players[curplayer] += curmarble.value;
			curmarble = curmarble.removeAndNext();
		}
		curplayer = (curplayer + 1) % numplayers;
	}

	ulong maxscore = 0;
	foreach(score; players) {
		if (maxscore < score) {
			maxscore = score;
		}
	}

	writeln("Top score: ", maxscore);

	return 0;
}
