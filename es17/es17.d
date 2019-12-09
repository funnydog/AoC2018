import std.stdio;
import std.format;

enum {
	NONE = 0,
	CLAY = 1,
	SAND = 2,
	WATER = 3,
}

struct Map {
	int xmin, xmax, ymin, ymax;
	int[][] map;
	int width, height;

	void minmax(int x, int y) {
		if (this.xmin > x) this.xmin = x;
		if (this.xmax < x) this.xmax = x;
		if (this.ymin > y) this.ymin = y;
		if (this.ymax < y) this.ymax = y;
	}

	void set(int x, int y, int value) {
		if (x >= this.xmin && x <= this.xmax &&
		    y >= this.ymin && y <= this.ymax) {
			this.map[y - this.ymin][x - this.xmin] = value;
		}
	}

	int get(int x, int y) {
		if (x >= this.xmin && x <= this.xmax &&
		    y >= this.ymin && y <= this.ymax) {
			return this.map[y - this.ymin][x - this.xmin];
		}
		return 0;
	}

	int open(int x, int y) {
		int v = this.get(x, y);
		return v == NONE || v == SAND;
	}

	void fill(int x, int y) {
		if (y >= this.ymin + this.height)
			return;
		else if (!this.open(x, y))
			return;

		if (!this.open(x, y+1)) {
			int l;
			for (l = x; this.open(l, y) && !this.open(l, y+1); l--) {
				this.set(l, y, SAND);
			}
			int r;
			for (r = x+1; this.open(r, y) && !this.open(r, y+1); r++) {
				this.set(r, y, SAND);
			}
			if (this.open(l, y+1) || this.open(r, y+1)) {
				this.fill(l, y);
				this.fill(r, y);
			} else {
				for (l = l+1; l < r; ++l) {
					this.set(l, y, WATER);
				}
				this.fill(x, y-1);
			}
		} else if (this.get(x, y) == NONE) {
			this.set(x, y, SAND);
			this.fill(x, y+1);
		}
	}

	void count(ref int sand, ref int water) {
		sand = water = 0;
		for (int y = 0; y < this.height; ++y) {
			for (int x = 0; x < this.width; ++x) {
				if (this.map[y][x] == SAND)
					++sand;
				else if (this.map[y][x] == WATER)
					++water;
			}
		}
	}

	void print() {
		stdout.writef("Map size %dx%d\n", this.width, this.height);
		for (int y = 0; y < this.height; ++y) {
			for (int x = 0; x < this.width; ++x) {
				switch(this.map[y][x]) {
				default:
				case NONE: stdout.write("."); break;
				case CLAY: stdout.write("#"); break;
				case SAND: stdout.write("|"); break;
				case WATER: stdout.write("~"); break;
				}
			}
			stdout.writeln();
		}
	}

	this(File input) {
		// compute the map size
		this.xmin = this.ymin = int.max;
		this.xmax = this.ymax = int.min;
		foreach(line; input.byLine()) {
			int a, b, c;
			if (line[0] == 'x') {
				line.formattedRead("x=%s, y=%s..%s", &a, &b, &c);
				this.minmax(a, b);
				this.minmax(a, c);
			} else if(line[0] == 'y') {
				line.formattedRead("y=%s, x=%s..%s", &a, &b, &c);
				this.minmax(b, a);
				this.minmax(c, a);
			}
		}
		++this.xmax;
		--this.xmin;

		// build the map
		this.width = this.xmax - this.xmin + 1;
		this.height = this.ymax - this.ymin + 1;
		this.map = new int[][](this.height, this.width);

		input.rewind();
		foreach(line; input.byLine) {
			int a, b, c;
			if (line[0] == 'x') {
				line.formattedRead("x=%s, y=%s..%s", &a, &b, &c);
				for (int y = b; y <= c; y++) {
					this.set(a, y, CLAY);
				}
			} else if(line[0] == 'y') {
				line.formattedRead("y=%s, x=%s..%s", &a, &b, &c);
				for (int x = b; x <= c; x++) {
					this.set(x, a, CLAY);
				}
			}
		}
	}
}

int main(string[] args) {
	if (args.length < 2) {
		stderr.writef("Usage: %s <filename>\n", args[0]);
		return -1;
	}

	File input;
	try {
		input = File(args[1], "rb");
	} catch (std.exception.ErrnoException) {
		stderr.writef("Cannot open file %s for reading\n", args[1]);
		return -1;
	}

	Map m = Map(input);
	input.close();
//	m.print();
	m.fill(500,0);
//	m.print();
	int sand, water;
	m.count(sand, water);
	writef("sand %d, water %d, sum %d\n", sand, water, sand+water);

	return 0;
}
