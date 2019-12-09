import std.stdio;
import std.format;

struct Light {
	long x, y;
	long vx, vy;

	this(long x, long y, long vx, long vy) {
		this.x = x;
		this.y = y;
		this.vx = vx;
		this.vy = vy;
	}

	void stepForward() {
		this.x += this.vx;
		this.y += this.vy;
	}

	void stepBackwards() {
		this.x -= this.vx;
		this.y -= this.vy;
	}
}

struct Extents {
	long xmin, ymin;
	long xmax, ymax;
}

long findArea(Light[] lights, Extents *r) {
	Extents e = { long.max, long.max, long.min, long.min };
	foreach (ref l; lights) {
		if (e.xmin > l.x) e.xmin = l.x;
		if (e.xmax < l.x) e.xmax = l.x;
		if (e.ymin > l.y) e.ymin = l.y;
		if (e.ymax < l.y) e.ymax = l.y;
	}
	if (r) {
		*r = e;
	}
	return (e.xmax - e.xmin + 1) * (e.ymax - e.ymin + 1);
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

	Light[] lights;
	try {
		long x, y, vx, vy;
		while (input.readf(" position=< %s, %s> velocity=< %s, %s>",
				   &x, &y, &vx, &vy)) {
			lights ~= Light(x, y, vx, vy);
		}
	} catch (std.format.FormatException) {
	} finally {
		input.close();
	}
	writef("Read %d light points\n", lights.length);

	long minarea = long.max;
	long minseconds;
	long curseconds;
	for (curseconds = 0; ; ++curseconds) {
		long area = findArea(lights, null);
		if (minarea < area) {
			break;
		} else if (minarea > area) {
			minarea = area;
			minseconds = curseconds;
		}
		foreach(ref l; lights) {
			l.stepForward();
		}
	}

	writef("Minimum reached after %d seconds, area %d\n",
	       minseconds, minarea);

	for (;curseconds > minseconds; --curseconds) {
		foreach(ref l; lights) {
			l.stepBackwards();
		}
	}

	Extents e;
	findArea(lights, &e);
	writef("Map size %dx%d starting at(%d, %d)\n",
	       e.xmax - e.xmin + 1, e.ymax - e.ymin + 1, e.xmin, e.ymin);

	for (long y = e.ymin; y <= e.ymax; y++) {
		for (long x = e.xmin; x <= e.xmax; x++) {
			bool found = false;
			foreach (ref l; lights) {
				if (l.x == x && l.y == y) {
					found = true;
					write("#");
					break;
				}
			}
			if (!found) {
				write(" ");
			}
		}
		writeln();
	}

	return 0;
}
