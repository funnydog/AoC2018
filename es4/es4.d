import std.algorithm;
import std.format;
import std.stdio;

enum EventType { BEGINS_SHIFT, FALLS_ASLEEP, WAKES_UP }

struct Event {
	int year;
	int month;
	int day;
	int hour;
	int minute;

	int guard_id;
	EventType type;

	this(string line) {
		formattedRead(line,
			      " [%s-%s-%s %s:%s]",
			      &this.year,
			      &this.month,
			      &this.day,
			      &this.hour,
			      &this.minute);

		try {
			formattedRead(line, " Guard #%s begins shift", &this.guard_id);
			this.type = EventType.BEGINS_SHIFT;
			return;
		} catch (FormatException e) {
		} catch (std.conv.ConvException e) {
		}

		try {
			formattedRead(line," falls asleep");
			this.type = EventType.FALLS_ASLEEP;
			this.guard_id = 0;
			return;
		} catch (FormatException e) {
		}

		try {
			formattedRead(line, " wakes up");
			this.guard_id = 0;
			this.type = EventType.WAKES_UP;
		} catch (FormatException e) {
		}
	}
}

bool event_cmp(ref Event a, ref Event b) {
	if (a.year != b.year)
		return a.year < b.year;
	if (a.month != b.month)
		return a.month < b.month;
	if (a.day != b.day)
		return a.day < b.day;
	if (a.hour != b.hour)
		return a.hour < b.hour;
	return a.minute < b.minute;
}

struct Guard {
	int id;
	int[60] minutes;
	int total_asleep;
	int max_asleep;
	int max_asleep_minute;

	this(int id) {
		this.id = id;
		this.minutes[] = 0;
	}

	void compute_asleep() {
		this.total_asleep = 0;
		this.max_asleep = -0;
		this.max_asleep_minute = -1;
		for (int i = 0; i < 60; i++) {
			this.total_asleep += this.minutes[i];
			if (this.max_asleep < this.minutes[i]) {
				this.max_asleep = this.minutes[i];
				this.max_asleep_minute = i;
			}
		}
	}

	void fill(int start, int end) {
		for (int i = start; i < end; ++i) {
			++this.minutes[i];
		}
	}
}

bool guard_total_asleep_cmp(Guard a, Guard b)
{
	return a.total_asleep > b.total_asleep;
}

bool guard_max_asleep_cmp(Guard a, Guard b)
{
	return a.max_asleep > b.max_asleep;
}

int main(string[] args) {
	if (args.length < 2) {
		stderr.writef("Usage: %s filename\n", args[0]);
		return -1;
	}

	File input = File(args[1], "r");
	if (!input.isOpen()) {
		stderr.writeln("File not found\n");
		return -1;
	}

	Event[] events;
	foreach (line; input.byLine) {
		events ~= Event(line.idup);
	}
	events.sort!(event_cmp);

	Guard[int] guard_map;
	int guard_id = 0;
	EventType last = EventType.BEGINS_SHIFT;
	int minute = 0;
	foreach(ref e; events) {
		if (e.type == EventType.BEGINS_SHIFT) {
			if (e.guard_id != guard_id && last == EventType.FALLS_ASLEEP) {
				guard_map[guard_id].fill(minute, 60);
			}
			guard_id = e.guard_id;
			guard_map.require(guard_id, Guard(guard_id));
		} else if (e.type == EventType.FALLS_ASLEEP) {
			minute = e.minute;
		} else if (e.type == EventType.WAKES_UP) {
			guard_map[guard_id].fill(minute, e.minute);
		}
		last = e.type;
	}

	Guard[] guards;
	foreach(guard; guard_map) {
		guard.compute_asleep();
		guards ~= guard;
	}

	guards.sort!(guard_total_asleep_cmp);
	writeln("Strategy1: ", guards[0].id * guards[0].max_asleep_minute);

	guards.sort!(guard_max_asleep_cmp);
	writeln("Strategy2: ", guards[0].id * guards[0].max_asleep_minute);

	return 0;
}
