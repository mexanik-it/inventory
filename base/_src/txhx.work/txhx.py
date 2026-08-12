# txhx.py
import os
import shutil
from typing import Optional, Dict, Any, List
from datetime import datetime

from paths import check_source_available, list_source_files, get_backup_path, build_push_items
from backup_engine import perform_backup
from progress import run_with_progress, ProgressBar
from report import append_report_row, init_report, finalize_report
from config import SOURCE_PATH, BACKUP_ROOT


class TXHX:
    """
    Класс TXHX для резервного копирования, восстановления и подготовки к сжатию.
    Разбит на модули: paths, backup_engine, progress, report, config.
    """

    def __init__(self, project_root: Optional[str] = None):
        """
        Инициализация: проверка доступности сетевого источника.
        project_root — корень проекта (где лежат list-points.* и папка XML).
        Если None — берётся текущая рабочая директория.
        """
        self.project_root = project_root or os.getcwd()
        self.source_ok, self.source_err = check_source_available()

        if not self.source_ok:
            # Можно выбросить исключение, но пока просто сохраняем статус
            pass

    def backup(self) -> Dict[str, Any]:
        """
        Выполняет резервное копирование:
          - копирует все файлы из сетевого пути SOURCE_PATH в папку backup/<дата_время>
          - дописывает list-points.xlsx, list-points.html и папку XML из проекта
          - ведёт отчёт в backup/report.html
          - показывает прогресс-бар с ETA
        Возвращает словарь с результатами: backup_dir, ok, fail, status.
        """
        if not self.source_ok:
            raise RuntimeError(f"Source unavailable: {self.source_err}")

        # Подсчёт приблизительного числа операций для прогресс-бара
        source_files = list_source_files()
        total_ops = len(source_files) + 3  # 2 файла + 1 папка из проекта

        result: Dict[str, Any] = {}

        def do_backup(pb: ProgressBar):
            res = perform_backup(self.project_root)
            result.update(res)
            # Условно завершаем прогресс (можно точнее считать внутри perform_backup)
            pb.increment(total_ops)

        run_with_progress(total_ops, "Backup", do_backup)

        print("\nBackup finished.")
        print(f"Backup stored in: {result.get('backup_dir', 'unknown')}")
        print(f"Report: {os.path.join(BACKUP_ROOT, 'report.html')}")
        print(f"Total: {result.get('ok', 0) + result.get('fail', 0)}, "
              f"OK: {result.get('ok', 0)}, Fail: {result.get('fail', 0)}")

        return result

    def restore(self, backup_folder: str) -> Dict[str, Any]:
        """
        Восстановление файлов из указанной папки backup.
        Копирует файлы и папки из backup_folder обратно в SOURCE_PATH.
        Отчёт пишется в тот же report.html (дозапись).
        Возвращает результат: status, ok, fail.
        """
        init_report()
        append_report_row("RESTORE_START", f"Restoring from {backup_folder}", "RUNNING")

        ok = 0
        fail = 0

        if not os.path.isdir(backup_folder):
            append_report_row("RESTORE", backup_folder, "FAIL: folder not found")
            return {"status": "FAIL", "ok": 0, "fail": 1}

        entries = os.listdir(backup_folder)
        total = len(entries)

        pb = ProgressBar(total, "Restore")
        pb.start()

        for name in entries:
            src = os.path.join(backup_folder, name)
            dst = os.path.join(SOURCE_PATH, name)

            try:
                if os.path.isfile(src):
                    shutil.copy2(src, dst)
                    append_report_row("RESTORE", f"{name}", "OK")
                    ok += 1
                elif os.path.isdir(src):
                    if os.path.exists(dst):
                        shutil.rmtree(dst)
                    shutil.copytree(src, dst, dirs_exist_ok=True)
                    append_report_row("RESTORE", f"{name}/", "OK")
                    ok += 1
                else:
                    append_report_row("RESTORE", f"{name}", "FAIL: unknown type")
                    fail += 1
            except Exception as e:
                append_report_row("RESTORE", f"{name} | Error: {e}", "FAIL")
                fail += 1

            pb.increment()

        pb.finish()

        status = "SUCCESS" if fail == 0 else "PARTIAL"
        append_report_row("RESTORE_FINISH", f"Total OK: {ok}, Fail: {fail}", status)
        finalize_report()

        print(f"\nRestore finished. Status: {status}")
        print(f"OK: {ok}, Fail: {fail}")

        return {"status": status, "ok": ok, "fail": fail}

    def compress(self, backup_folder: str, output_zip: Optional[str] = None) -> Dict[str, Any]:
        """
        Заготовка для метода сжатия резервной копии в ZIP.
        Сейчас выполняет только проверку и логирование; реальная архивация — позже.
        output_zip — имя выходного ZIP-файла. Если None, формируется автоматически.
        Возвращает: status, archive_path, ok, fail.
        """
        init_report()
        append_report_row("COMPRESS_START", f"Compressing {backup_folder}", "RUNNING")

        if not os.path.isdir(backup_folder):
            append_report_row("COMPRESS", backup_folder, "FAIL: folder not found")
            finalize_report()
            return {"status": "FAIL", "archive_path": None, "ok": 0, "fail": 1}

        if output_zip is None:
            base_name = os.path.basename(backup_folder.rstrip(os.sep))
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            output_zip = f"{base_name}_{timestamp}.zip"
            output_zip = os.path.join(BACKUP_ROOT, output_zip)

        try:
            # Здесь будет реальная архивация (например, через shutil.make_archive)
            # Пока просто логируем намерение
            append_report_row(
                "COMPRESS",
                f"Would create archive: {output_zip}",
                "PENDING"
            )
            status = "PENDING"
            ok = 1
            fail = 0
        except Exception as e:
            append_report_row("COMPRESS", f"Error: {e}", "FAIL")
            status = "FAIL"
            ok = 0
            fail = 1

        append_report_row("COMPRESS_FINISH", f"Archive path: {output_zip}", status)
        finalize_report()

        print(f"\nCompress status: {status}, archive: {output_zip}")
        return {
            "status": status,
            "archive_path": output_zip,
            "ok": ok,
            "fail": fail,
        }

    def remove_duplicate_lines(self, file_path: str) -> bool:
        """
        Вспомогательный метод для удаления повторяющихся строк в текстовом файле.
        Используется как заготовка под будущую функциональность «удаления дубликатов».
        Возвращает True при успехе, иначе False.
        """
        try:
            with open(file_path, "r", encoding="utf-8") as f:
                lines = f.readlines()

            unique_lines = []
            seen = set()
            for line in lines:
                stripped = line.rstrip("\n\r")
                if stripped not in seen:
                    seen.add(stripped)
                    unique_lines.append(line)

            with open(file_path, "w", encoding="utf-8") as f:
                f.writelines(unique_lines)

            append_report_row("REMOVE_DUPS", file_path, "OK")
            return True
        except Exception as e:
            append_report_row("REMOVE_DUPS", f"{file_path} | Error: {e}", "FAIL")
            return False
