import std.stdio;
import std.conv;

struct Grid {
	int[301][301] data;

	this(int serial) {
		for (int y = 1; y < 301; y++) {
			for (int x = 1; x < 301; x++) {
				int rack_id = x + 10;
				int powerlevel = rack_id * y;
				powerlevel += serial;
				powerlevel *= rack_id;
				powerlevel = ((powerlevel/100) % 10) - 5;

				// summer-area table
				this.data[y][x] = powerlevel +
					this.data[y][x-1] +
					this.data[y-1][x] -
					this.data[y-1][x-1];
			}
		}
	}

	int getSum(int x, int y, int s) {
		return this.data[y][x] + this.data[y+s][x+s] -
			this.data[y+s][x] - this.data[y][x+s];
	}

	int findLargestBlock(ref int px, ref int py, int side) {
		px = 1;
		py = 1;
		int pp = this.data[1][1];
		for (int y = 1; y < 301-side; y++) {
			for (int x = 1; x < 301-side; x++) {
				int p = this.getSum(x, y, side);
				if (pp < p) {
					pp = p;
					px = x + 1;
					py = y + 1;
				}
			}
		}
		return pp;
	}

	int findLargestBlockAnySide(ref int px, ref int py, ref int side) {
		int pp = int.min;
		for (int s = 1; s <= 300; s++) {
			int x, y, p;
			p = this.findLargestBlock(x, y, s);
			if (pp < p) {
				pp = p;
				px = x;
				py = y;
				side = s;
			}
		}
		return pp;
	}
}

int main(string[] args) {
	if (args.length < 2) {
		stderr.writef("Usage: %s <serial>\n", args[0]);
		return -1;
	}

	int serial;
	try {
		serial = to!int(args[1]);
	} catch (std.conv.ConvException) {
		stderr.writef("Cannot parse the serial %s\n", args[1]);
		return -1;
	}

	Grid grid = Grid(serial);
	int x, y, s, p;

	p = grid.findLargestBlock(x, y, 3);
	writef("Largest total power: %d at %d,%d\n", p, x, y);

	p = grid.findLargestBlockAnySide(x, y, s);
	writef("Largest total power: %d at %d,%d,%d\n", p, x, y, s);

	return 0;
}
