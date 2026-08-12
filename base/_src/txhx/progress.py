# progress.py
from typing import Callable, Optional
from datetime import timedelta


class ProgressBar:
    def __init__(self, total: int, description: str = "Progress"):
        self.total = total
        self.description = description
        self.count = 0
        self._start_time = None
        self._last_print = 0

    def start(self) -> None:
        import time
        self._start_time = time.time()
        self._print()

    def increment(self, n: int = 1) -> None:
        self.count += n
        self._print()

    @property
    def percent(self) -> float:
        if self.total == 0:
            return 0.0
        return (self.count / self.total) * 100

    @property
    def eta_seconds(self) -> Optional[float]:
        import time
        if not self._start_time or self.count == 0:
            return None
        elapsed = time.time() - self._start_time
        rate = self.count / elapsed if elapsed > 0 else 0
        remaining = self.total - self.count
        return remaining / rate if rate > 0 else None

    @property
    def eta(self) -> str:
        sec = self.eta_seconds
        if sec is None:
            return "?"
        td = timedelta(seconds=int(sec))
        hours, remainder = divmod(int(td.total_seconds()), 3600)
        minutes, seconds = divmod(remainder, 60)
        return f"{hours:02d}:{minutes:02d}:{seconds:02d}"

    def _print(self) -> None:
        bar_len = 40
        filled = int(bar_len * self.percent / 100)
        bar = "█" * filled + "░" * (bar_len - filled)
        eta_str = self.eta
        line = (
            f"\r{self.description} |{bar}| "
            f"{self.percent:6.2f}% | ETA: {eta_str} | "
            f"Done: {self.count}/{self.total}"
        )
        print(line, end="", flush=True)

    def finish(self) -> None:
        print()  # перевод строки после прогресс-бара


def run_with_progress(
    total: int,
    description: str,
    callback: Callable[[ProgressBar], None],
) -> None:
    pb = ProgressBar(total, description)
    pb.start()
    callback(pb)
    pb.finish()
