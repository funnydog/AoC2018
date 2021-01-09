import std.algorithm;
import std.exception: ErrnoException;
import std.format;
import std.stdio;

enum EventType { BEGINS_SHIFT, FALLS_ASLEEP, WAKES_UP };

struct Event
{
	int year;
	int month;
	int day;
	int hour;
	int minute;

	int guard_id;

	EventType type;

	this(string line)
	{
		formattedRead(line,
			      "[%s-%s-%s %s:%s] ",
			      &this.year,
			      &this.month,
			      &this.day,
			      &this.hour,
			      &this.minute);
		this.guard_id = -1;
		if (line.startsWith("falls asleep"))
		{
			this.type = EventType.FALLS_ASLEEP;
		}
		else if (line.startsWith("wakes up"))
		{
			this.type = EventType.WAKES_UP;
		}
		else
		{
			formattedRead(line, " Guard #%s begins shift", &this.guard_id);
			this.type = EventType.BEGINS_SHIFT;
		}
	}
}

bool event_cmp(ref Event a, ref Event b)
{
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

struct Guard
{
	int id;
	int[60] minutes;
	int total_asleep;
	int max_asleep;
	int max_asleep_minute;

	this(int id)
	{
		this.id = id;
		this.minutes[] = 0;
	}

	void compute_asleep()
	{
		this.total_asleep = 0;
		this.max_asleep = 0;
		this.max_asleep_minute = -1;
		for (int i = 0; i < 60; i++) {
			this.total_asleep += this.minutes[i];
			if (this.max_asleep < this.minutes[i]) {
				this.max_asleep = this.minutes[i];
				this.max_asleep_minute = i;
			}
		}
	}

	void fill(int start, int end)
	{
		for (int i = start; i < end; i++)
		{
			this.minutes[i]++;
		}
	}
}

Guard[] load(ref File input)
{
	Event[] events;
	foreach (line; input.byLine)
	{
		events ~= Event(line.idup);
	}
	events.sort!(event_cmp);

	Guard[int] guard_map;
	int guard_id = -1;
	Event *last = null;
	foreach(ref e; events)
	{
		switch(e.type)
		{
		case EventType.BEGINS_SHIFT:
			guard_id = e.guard_id;
			guard_map.require(guard_id, Guard(guard_id));
			last = null;
			break;

		case EventType.FALLS_ASLEEP:
			last = &e;
			break;

		case EventType.WAKES_UP:
			if (last)
			{
				guard_map[guard_id].fill(last.minute, e.minute);
				last = null;
			}
			break;

		default:
			assert(0);
		}
	}

	Guard[] guards;
	foreach(guard; guard_map)
	{
		guard.compute_asleep();
		guards ~= guard;
	}

	return guards;
}

bool guard_total_asleep_cmp(Guard a, Guard b)
{
	return a.total_asleep > b.total_asleep;
}

bool guard_max_asleep_cmp(Guard a, Guard b)
{
	return a.max_asleep > b.max_asleep;
}

int main(string[] args)
{
	if (args.length < 2)
	{
		stderr.writefln("Usage: %s <filename>", args[0]);
		return 1;
	}

	Guard[] guards;
	try
	{
		File input = File(args[1], "r");
		guards = load(input);
		input.close();
	}
	catch (ErrnoException e)
	{
		stderr.writefln("Cannot open %s", args[1]);
		return 1;
	}

	guards.sort!(guard_total_asleep_cmp);
	writefln("Part1: %d", guards[0].id * guards[0].max_asleep_minute);

	guards.sort!(guard_max_asleep_cmp);
	writefln("Part2: %d ", guards[0].id * guards[0].max_asleep_minute);

	return 0;
}
