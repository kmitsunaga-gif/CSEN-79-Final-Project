# OUTPUT\_FORMAT.md

Every program prints exactly the lines below and nothing else, so a test is a `diff` between your output and the expected output. This file ships in the starter kit.

## Rules for both projects

- One item per line, fields separated by a single space, no trailing spaces, Unix line endings.
- Prices always have two decimals (`50.10`, `579.00`); quantities are whole numbers.
- Commands not listed as printing something print nothing.
- An invalid command prints one line, `ERROR <command number> <reason>`, and changes nothing. The command number counts commands from 1. Reasons: `unknown-command`, `bad-arguments`, `duplicate-id`, `unknown-id`, `not-subscribed`.
- `STATS` lines all start with `STATS`, and their counter names are yours to choose. Test scripts drop them before comparing: `grep -v '^STATS'`.
- With `--quiet`, only `STATS` lines are printed.

## Project 1: Order book

| Command | Prints |
| --- | --- |
| `BUY`, `SELL` | One `TRADE <buy id> <sell id> <qty> <price>` per fill, in fill order, at the resting order's price; then `REST <id> <qty> <price>` if anything is left over |
| `MARKET` | One `TRADE` line per fill; then `UNFILLED <id> <qty>` if the book ran out (market orders never rest) |
| `CANCEL <id>` | `CANCELLED <id> <remaining qty>` |
| `BOOK [depth]` | Up to `depth` ask levels, highest price first, then `---`, then up to `depth` bid levels, highest first. Each level: `ASK <price> <total qty> <order count>` or `BID …`. Default depth is 5. |
| `HISTORY <user> [n]` | Oldest first: `<command number> ORDER <id> BUY\|SELL <qty> <price or MKT>`, `<command number> FILL <id> <qty> <price>`, `<command number> CANCEL <id> <qty>` |

Example:

```
SELL ann 1 100 50.20
SELL ann 2 200 50.30
BUY bob 3 250 50.30
BOOK
```

prints

```
REST 1 100 50.20
REST 2 200 50.30
TRADE 3 1 100 50.20
TRADE 3 2 150 50.30
ASK 50.30 50 1
---
```

## Project 2: Alert service

Times print as `D<day> <hh:mm>`, starting at `D1 00:00`.

| Command | Prints |
| --- | --- |
| `PUB`, `PRICE`, `TICK` | One `ALERT <time> <user> <topic> <message>` per delivery. A price alert's message is `price <new price>`. Alerts delivered at the same time print sorted by user name, then in publish order. |
| `TICK <minutes>` | Its alerts, then `TIME <new time>` |
| `INBOX <user> [n]` | Newest first: `<time> <topic> <message>` |
| `HISTORY <user> [n]` | Oldest first: `<time> SUB <topic> [options as typed]`, `<time> UNSUB <topic>`, `<time> ALERT <topic> <message>` |

Sorting by user name is only needed for printed output. Under `--quiet` nothing is printed, so it costs nothing in benchmarks.

Example, the script from Project 2's "How input works", prints

```
ALERT D1 00:00 bob items/phones/pixel-10 price 579.00
ALERT D1 08:00 alice news/sports/soccer Arsenal win 2-1
TIME D1 08:00
D1 08:00 news/sports/soccer Arsenal win 2-1
D1 00:00 items/phones/pixel-10 price 579.00
```

followed by the `STATS` lines. Carol's `INBOX` prints nothing because it is empty.
