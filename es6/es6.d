import std.stdio;

struct location {
	int id;
	int x;
	int y;
}

int distance(int x0, int y0, int x1, int y1) {
	return ((x1 > x0) ? (x1 - x0) : (x0 - x1)) +
		((y1 > y0) ? (y1 - y0) : (y0 - y1));
}

struct region {
	int x;
	int y;
	int w;
	int h;

	int[][] map;

	this(location[] locs) {
		int xmin, xmax;
		int ymin, ymax;
		xmin = ymin = int.max;
		xmax = ymax = int.min;
		foreach(ref l; locs) {
			if (l.x < xmin) xmin = l.x;
			if (l.x > xmax) xmax = l.x;
			if (l.y < ymin) ymin = l.y;
			if (l.y > ymax) ymax = l.y;
		}

		this.x = xmin;
		this.y = ymin;
		this.w = xmax - xmin + 1;
		this.h = ymax - ymin + 1;

		this.map = new int[][](this.h, this.w);
	}

	void partition(location[] locs) {
		for (int y = 0; y < this.h; y++) {
			for (int x = 0; x < this.w; x++) {
				int min = int.max;
				int minlabel = -1;
				foreach (ref l; locs) {
					int d = distance(
						x + this.x, y + this.y,
						l.x, l.y);
					if (d == min) {
						minlabel = -1;
					} else if (d < min) {
						min = d;
						minlabel = l.id;
					}
				}
				this.map[y][x] = minlabel;
			}
		}
	}

	int find_area(ref location loc) {
		int area = 0;
		for (int y = 0; y < this.h; y++) {
			for (int x = 0; x < this.w; x++) {
				if (loc.id == this.map[y][x]) {
					if (x == 0 || x == this.w - 1 ||
					    y == 0 || y == this.h - 1)
						return 0;
					area++;
				}
			}
		}
		return area;
	}

	int find_max_area(location[] locs) {
		this.partition(locs);
		int maxarea = int.min;
		foreach (ref loc; locs) {
			int area = this.find_area(loc);
			if (area > maxarea) {
				maxarea = area;
			}
		}
		return maxarea;
	}

	void compute_distance_sums(location[] locs) {
		for (int y = 0; y < this.h; y++) {
			for (int x = 0; x < this.w; x++) {
				int sum = 0;
				foreach (ref loc; locs) {
					sum += distance(
						x + this.x, y + this.y,
						loc.x, loc.y);
				}
				this.map[y][x] = sum;
			}
		}
	}

	int find_area_lt(int limit) {
		int area = 0;
		for (int y = 0; y < this.h; y++) {
			for (int x = 0; x < this.w; x++) {
				if (this.map[y][x] < limit) {
					++area;
				}
			}
		}
		return area;
	}
}

int main(string[] args) {
	if (args.length < 2) {
		stderr.writef("Usage: %s filename\n", args[0]);
		return -1;
	}

	File input;
	try  {
		input = File(args[1], "r");
	} catch (std.exception.ErrnoException) {
		stderr.writef("Cannot open %s for reading\n", args[1]);
		return -1;
	}

	location loc;
	location[] locs;
	while (input.readf(" %s, %s", &loc.x, &loc.y)) {
		loc.id = cast(int)locs.length;
		locs ~= loc;
	}
	input.close();

	region r = region(locs);
	writeln("Largest area that isn't infinite: ", r.find_max_area(locs));
	r.compute_distance_sums(locs);
	writeln("Size of the region with distance sum < 32: ", r.find_area_lt(32));
	writeln("Size of the region with distance sum < 10000: ", r.find_area_lt(10000));

	return 0;
}
