// src/ncurses_display.cpp
#include "ncurses_display.h"
#include "format.h"
#include "system.h"
#include "process.h"

#include <ncurses.h>
#include <unistd.h>   // getpid(), usleep
#include <signal.h>   // kill, SIGKILL, SIGSTOP, SIGCONT
#include <string>
#include <vector>
#include <chrono>
#include <thread>
using namespace Format;

namespace NCursesDisplay {

void Display(System& system, int n /*unused, kept for compatibility*/) {
  // Initialize ncurses
  initscr();
  noecho();
  cbreak();
  curs_set(0);
  keypad(stdscr, TRUE);
  start_color();

  // color pairs (may vary by terminal)
  init_pair(1, COLOR_WHITE, COLOR_BLACK);
  init_pair(2, COLOR_YELLOW, COLOR_BLACK);
  init_pair(3, COLOR_RED, COLOR_BLACK);
  init_pair(4, COLOR_GREEN, COLOR_BLACK);
  init_pair(5, COLOR_BLACK, COLOR_WHITE); // footer reverse

  pid_t self_pid = getpid();
  int selected = 0;
  std::string status_msg = "";

  while (true) {
    // get terminal size
    int term_h, term_w;
    getmaxyx(stdscr, term_h, term_w);

    // compute window sizes
    int sys_h = 8;                         // top system panel height
    int footer_h = 2;                      // footer + status
    int proc_h = term_h - sys_h - footer_h;
    if (proc_h < 4) proc_h = 4;

    // clear and draw top system area directly on stdscr
    werase(stdscr);

    // Top panel box
    int box_left = 0, box_top = 0, box_w = term_w - 1;
    // draw the top box border
    for (int x = box_left; x <= box_w; ++x) {
      mvaddch(box_top, x, ACS_HLINE);
      mvaddch(box_top + sys_h, x, ACS_HLINE);
    }
    for (int y = box_top; y <= box_top + sys_h; ++y) {
      mvaddch(y, box_left, ACS_VLINE);
      mvaddch(y, box_w, ACS_VLINE);
    }
    mvaddch(box_top, box_left, ACS_ULCORNER);
    mvaddch(box_top, box_w, ACS_URCORNER);
    mvaddch(box_top + sys_h, box_left, ACS_LLCORNER);
    mvaddch(box_top + sys_h, box_w, ACS_LRCORNER);

    // System info: left column text
    int row = 1;
    mvprintw(row++, 2, "OS: %s", system.OperatingSystem().c_str());
    mvprintw(row++, 2, "Kernel: %s", system.Kernel().c_str());
    mvprintw(row++, 2, "CPU: ");
    // CPU progress text on same row at col 10
    attron(COLOR_PAIR(4));
    mvprintw(row - 1, 10, "%s", ProgressBar(system.Cpu().Utilization()).c_str());
    attroff(COLOR_PAIR(4));

    mvprintw(row++, 2, "Memory: ");
    attron(COLOR_PAIR(4));
    mvprintw(row - 1, 10, "%s", ProgressBar(system.MemoryUtilization()).c_str());
    attroff(COLOR_PAIR(4));
    mvprintw(row++, 2, "Total Processes: %d", system.TotalProcesses());
    mvprintw(row++, 2, "Running Processes: %d", system.RunningProcesses());
    mvprintw(row++, 2, "Up Time: %s", Format::ElapsedTime(system.UpTime()).c_str());

    // Process area box (below top panel)
    int proc_box_top = sys_h + 1;
    for (int x = 0; x <= box_w; ++x) {
      mvaddch(proc_box_top - 1, x, ACS_HLINE);
      mvaddch(proc_box_top + proc_h, x, ACS_HLINE);
    }
    for (int y = proc_box_top - 1; y <= proc_box_top + proc_h; ++y) {
      mvaddch(y, 0, ACS_VLINE);
      mvaddch(y, box_w, ACS_VLINE);
    }
    mvaddch(proc_box_top - 1, 0, ACS_ULCORNER);
    mvaddch(proc_box_top - 1, box_w, ACS_URCORNER);
    mvaddch(proc_box_top + proc_h, 0, ACS_LLCORNER);
    mvaddch(proc_box_top + proc_h, box_w, ACS_LRCORNER);

    // Header row inside process area
    int header_y = proc_box_top;
    mvprintw(header_y, 2, "PID");
    mvprintw(header_y, 9, "USER");
    mvprintw(header_y, 22, "CPU");
    mvprintw(header_y, 30, "RAM");
    mvprintw(header_y, 38, "UPTIME");
    mvprintw(header_y, 48, "ST");
    mvprintw(header_y, 52, "COMMAND");

    // Get processes (updates internally)
    std::vector<Process>& procs = system.Processes().GetProcesses();

    // clamp selected index
    if (procs.empty()) selected = 0;
    else if (selected >= (int)procs.size()) selected = procs.size() - 1;
    else if (selected < 0) selected = 0;

    // compute how many rows inside box for processes
    int rows_available = proc_h - 2; // leave header and bottom border
    if (rows_available < 1) rows_available = 1;

    // simple paging: ensure selected visible
    int top_index = 0;
    if (selected >= rows_available) top_index = selected - rows_available + 1;

    for (int i = 0; i < rows_available && (top_index + i) < (int)procs.size(); ++i) {
      int idx = top_index + i;
      int y = header_y + 1 + i;
      // highlight selected
      if (idx == selected) attron(A_REVERSE);

      // choose color by state
      std::string st = procs[idx].State();
      if (!st.empty() && st[0] == 'R') attron(COLOR_PAIR(1));
      else if (!st.empty() && st[0] == 'S') attron(COLOR_PAIR(2));
      else if (!st.empty() && st[0] == 'T') attron(COLOR_PAIR(3));
      else attron(COLOR_PAIR(1));

      // fields
      mvprintw(y, 2, "%d", procs[idx].Pid());
      mvprintw(y, 9, "%-10s", procs[idx].User().substr(0,10).c_str());
      mvprintw(y, 22, "%5.2f", procs[idx].CpuUtilization() * 100.0);
      mvprintw(y, 30, "%6s", procs[idx].Ram().c_str());
      mvprintw(y, 38, "%8s", Format::ElapsedTime(procs[idx].UpTime()).c_str());
      mvprintw(y, 48, "%2s", procs[idx].State().substr(0,2).c_str());

      // command (truncate to fit)
      int cmd_col = 52;
      int cmd_width = term_w - cmd_col - 2;
      if (cmd_width < 10) cmd_width = 10;
      std::string cmd_shown = procs[idx].Command();
      if ((int)cmd_shown.size() > cmd_width) cmd_shown = cmd_shown.substr(0, cmd_width - 3) + "...";
      mvprintw(y, cmd_col, "%s", cmd_shown.c_str());

      // turn off colors/highlight
      attroff(COLOR_PAIR(1));
      attroff(COLOR_PAIR(2));
      attroff(COLOR_PAIR(3));
      if (idx == selected) attroff(A_REVERSE);
    }

    // Footer: status line + help line at very bottom
    int footer_help_y = term_h - 1;
    int footer_status_y = term_h - 2;

    // Help
    attron(A_REVERSE);
    mvprintw(footer_help_y, 0, "%-*s", term_w, "↑/↓: Navigate  k: Kill  s: Suspend  r: Resume  q: Quit");
    attroff(A_REVERSE);

    // Status message
    mvprintw(footer_status_y, 0, "%-*s", term_w, status_msg.c_str());

    // Refresh screen
    refresh();

    // ===== input handling (blocking) =====
    int ch = getch();
    if (ch == KEY_UP) {
      if (!procs.empty()) selected = (selected == 0) ? (int)procs.size() - 1 : selected - 1;
    } else if (ch == KEY_DOWN) {
      if (!procs.empty()) selected = (selected + 1 >= (int)procs.size()) ? 0 : selected + 1;
    } else if (ch == 'q' || ch == 'Q') {
      break;
    } else if ((ch == 'k' || ch == 'K') && !procs.empty()) {
      int target = procs[selected].Pid();
      if (target == self_pid) {
        status_msg = "Refusing to kill the monitor process.";
      } else if (kill(target, SIGKILL) == 0) {
        status_msg = "Killed PID " + std::to_string(target) + ".";
      } else {
        status_msg = "Failed to kill PID " + std::to_string(target) + " (permission or gone).";
      }
      // Immediately update list
      system.Processes().GetProcesses(); // GetProcesses() calls UpdateProcesses()
      if (selected >= (int)system.Processes().GetProcesses().size())
        selected = (int)system.Processes().GetProcesses().size() - 1;
      if (selected < 0) selected = 0;
    } else if ((ch == 's' || ch == 'S') && !procs.empty()) {
      int target = procs[selected].Pid();
      if (target == self_pid) {
        status_msg = "Refusing to suspend the monitor process.";
      } else if (kill(target, SIGSTOP) == 0) {
        status_msg = "Suspended PID " + std::to_string(target) + ".";
      } else {
        status_msg = "Failed to suspend PID " + std::to_string(target) + " (permission or gone).";
      }
      system.Processes().GetProcesses();
    } else if ((ch == 'r' || ch == 'R') && !procs.empty()) {
      int target = procs[selected].Pid();
      if (kill(target, SIGCONT) == 0) {
        status_msg = "Resumed PID " + std::to_string(target) + ".";
      } else {
        status_msg = "Failed to resume PID " + std::to_string(target) + " (permission or gone).";
      }
      system.Processes().GetProcesses();
    } else {
      // Unknown key: ignore
    }

    // small delay (not required since getch is blocking, but keep UI smooth)
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  } // while

  // cleanup
  endwin();
}

}  // namespace NCursesDisplay
