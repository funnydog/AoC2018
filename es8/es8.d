import std.stdio;
import std.format;

struct Node {
	Node*[] children;
	int[] metadata;

	int findLicense1() {
		int sum = 0;
		foreach(m; this.metadata) {
			sum += m;
		}
		foreach(c; this.children) {
			sum += c.findLicense1();
		}
		return sum;
	}

	int findLicense2() {
		int sum = 0;
		if (this.children.length == 0) {
			foreach(m; this.metadata) {
				sum += m;
			}
		} else {
			foreach(m; this.metadata) {
				if (m > 0 && m <= this.children.length) {
					sum += this.children[m-1].findLicense2();
				}
			}
		}
		return sum;
	}
}

Node* readNode(ref File input)
{
	Node *n = new Node;
	int ccount, mcount;

	try {
		input.readf(" %s %s", &ccount, &mcount);
	} catch (FormatException) {
		return null;
	}
	n.children = new Node*[ccount];
	foreach (ref c; n.children) {
		c = readNode(input);
	}

	n.metadata = new int[mcount];
	foreach (ref m; n.metadata) {
		input.readf(" %s", &m);
	}

	return n;
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

	Node* root = readNode(input);
	input.close();

	writeln("License1: ", root.findLicense1());
	writeln("License2: ", root.findLicense2());
	return 0;
}
