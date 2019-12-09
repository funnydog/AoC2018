import std.stdio;

struct point {
	int[4] v;

	// for disjoint-set / merge-find
	size_t parent;
	ulong rank;
}

int distance(point *a, point *b) {
	int r = 0;
	for (int i = 0; i < 4; i++) {
		r += (a.v[i] > b.v[i]) ? (a.v[i] - b.v[i]) : (b.v[i] - a.v[i]);
	}
	return r;
}

struct sky {
	point[] p;

	this(File input) {
		try {
			point p;
			while (input.readf(" %d,%d,%d,%d", &p.v[0], &p.v[1], &p.v[2], &p.v[3]) == 4) {
				p.parent = this.p.length;
				p.rank = 0;
				this.p ~= p;
			}
		} catch (std.format.FormatException) {
		}
	}

	size_t find(size_t i) {
		while (p[i].parent != i) {
			size_t next = p[i].parent;
			p[i].parent = p[next].parent;
			i = next;
		}
		return i;
	}

	void merge(size_t i, size_t j) {
		i = find(i);
		j = find(j);

		if (i == j)
			return;

		if (p[i].rank < p[j].rank) {
			p[i].parent = j;
		} else if (p[i].rank > p[j].rank) {
			p[j].parent = i;
		} else {
			p[i].parent = j;
			p[j].rank++;
		}
	}

	size_t constellations_count() {
		for (size_t i = 1; i < this.p.length; ++i) {
			for (size_t j = 0; j < i; ++j) {
				if (distance(&this.p[i], &this.p[j]) <= 3)
					merge(i, j);
			}
		}
		size_t count = 0;
		for (size_t i = 0; i < this.p.length; i++) {
			if (this.p[i].parent == i)
				count++;
		}
		return count;
	}
};



int main(string[] args) {
	if (args.length < 2) {
		stderr.writef("Usage: %s <filename>\n", args[0]);
		return -1;
	}

	File input;
	try {
		input = File(args[1], "rb");
	} catch (std.exception.ErrnoException) {
		stderr.writef("Cannot open %s for reading\n", args[1]);
		return -1;
	}

	sky s = sky(input);
	input.close();

	writef("Number of constellations: %d\n", s.constellations_count());
	return 0;
}
