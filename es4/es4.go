package main

import (
	"bufio"
	"fmt"
	"os"
	"sort"
)

type EventType int

const (
	BEGINS_SHIFT EventType = iota
	FALLS_ASLEEP
	WAKES_UP
)

type Event struct {
	Year    int
	Month   int
	Day     int
	Hour    int
	Minute  int
	Type    EventType
	GuardId int
}

func GetEvent(line string) (Event, error) {
	var e Event
	_, err := fmt.Sscanf(line, "[%d-%d-%d %d:%d] Guard #%d begins shift",
		&e.Year, &e.Month, &e.Day, &e.Hour, &e.Minute, &e.GuardId)
	if err == nil {
		e.Type = BEGINS_SHIFT
		return e, nil
	}

	_, err = fmt.Sscanf(line, "[%d-%d-%d %d:%d] falls asleep",
		&e.Year, &e.Month, &e.Day, &e.Hour, &e.Minute)
	if err == nil {
		e.GuardId = 0
		e.Type = FALLS_ASLEEP
		return e, nil
	}

	_, err = fmt.Sscanf(line, "[%d-%d-%d %d:%d] wakes up",
		&e.Year, &e.Month, &e.Day, &e.Hour, &e.Minute)
	if err == nil {
		e.GuardId = 0
		e.Type = WAKES_UP
		return e, nil
	}

	return e, err
}

type Guard struct {
	Id              int
	Minutes         [60]int
	TotalAsleep     int
	MaxAsleep       int
	MaxAsleepMinute int
}

func (guard *Guard) Compute() {
	guard.TotalAsleep = 0
	guard.MaxAsleep = 0
	guard.MaxAsleepMinute = -1
	for i, m := range guard.Minutes {
		guard.TotalAsleep += m
		if guard.MaxAsleep < m {
			guard.MaxAsleep = m
			guard.MaxAsleepMinute = i
		}
	}
}

func (guard *Guard) Fill(start, end int) {
	for i := start; i < end; i++ {
		guard.Minutes[i]++
	}
}

func main() {
	input, err := os.Open("input.txt")
	if err != nil {
		panic(err)
	}
	defer input.Close()

	events := []Event{}
	scanner := bufio.NewScanner(input)
	for scanner.Scan() {
		line := scanner.Text()
		e, err := GetEvent(line)
		if err != nil {
			panic(err)
		}

		events = append(events, e)
	}

	sort.Slice(events, func(i, j int) bool {
		if events[i].Year != events[j].Year {
			return events[i].Year < events[j].Year
		}
		if events[i].Month != events[j].Month {
			return events[i].Month < events[j].Month
		}
		if events[i].Day != events[j].Day {
			return events[i].Day < events[j].Day
		}
		if events[i].Hour != events[j].Hour {
			return events[i].Hour < events[j].Hour
		}
		return events[i].Minute < events[j].Minute
	})

	guard_map := map[int]*Guard{}
	lastState := BEGINS_SHIFT
	guardId := 0
	minute := 0
	for _, e := range events {
		switch e.Type {
		case BEGINS_SHIFT:
			if guardId != e.GuardId && lastState == FALLS_ASLEEP {
				guard_map[guardId].Fill(minute, 60)
			}
			guardId = e.GuardId
			_, ok := guard_map[guardId]
			if !ok {
				guard_map[guardId] = &Guard{Id: guardId}
			}

		case FALLS_ASLEEP:
			minute = e.Minute

		case WAKES_UP:
			guard_map[guardId].Fill(minute, e.Minute)
		}
		lastState = e.Type
	}

	guards := []*Guard{}
	for _, guard := range guard_map {
		guard.Compute()
		guards = append(guards, guard)
	}

	sort.Slice(guards, func(i, j int) bool {
		return guards[i].TotalAsleep > guards[j].TotalAsleep
	})
	fmt.Println("Strategy1", guards[0].Id*guards[0].MaxAsleepMinute)

	sort.Slice(guards, func(i, j int) bool {
		return guards[i].MaxAsleep > guards[j].MaxAsleep
	})
	fmt.Println("Strategy2", guards[0].Id*guards[0].MaxAsleepMinute)
}
