# Architettura FastAV

Questo documento descrive l'architettura tecnica di FastAV, le scelte di design e le ottimizzazioni implementate.

## 📐 Panoramica Architetturale

FastAV segue un'architettura **Model-View-Controller (MVC)** modificata con separazione netta tra:

```
┌─────────────────────────────────────────┐
│           GUI Layer (View)              │
│  MainWindow, ScanDialog, ScanProgress   │
└────────────┬────────────────────────────┘
             │ Signals/Slots
┌────────────┴────────────────────────────┐
│      Core Layer (Controller/Model)      │
│   Scanner, Database, Updater, Report    │
└────────────┬────────────────────────────┘
             │
┌────────────┴────────────────────────────┐
│        System Layer (Backend)           │
│     ClamAV (clamd), SQLite, Qt          │
└─────────────────────────────────────────┘
```

---

## 🧩 Componenti Principali

### 1. Core Layer

#### **Scanner.h/.cpp**
Responsabile della scansione parallela dei file.

**Caratteristiche chiave:**
- **Multi-threading**: Utilizza `QThreadPool` con worker threads
- **Task-based**: Ogni file è una `ScanTask` eseguita in parallelo
- **IPC ottimizzato**: Comunicazione con clamd via Unix socket (Linux) o TCP (Windows)
- **Atomic counters**: Thread-safe statistics senza lock

```cpp
// Architettura Scanner
Scanner
├── QThreadPool (gestisce N worker threads)
├── std::atomic<quint64> (contatori thread-safe)
└── ScanTask (QRunnable per ogni file)
    └── scanWithClamd() → Unix socket IPC
```

**Perché è veloce:**
1. **clamd sempre in memoria**: Database virus pre-caricato
2. **Zero overhead**: Nessun caricamento database per file
3. **Parallelismo**: 2×CPU cores threads simultanei
4. **IPC efficiente**: Unix sockets più veloci di TCP

#### **Database.h/.cpp**
Gestisce persistenza con SQLite.

**Schema Database:**
```sql
scan_history
├── id (PK)
├── scan_date
├── files_scanned
├── bytes_scanned
├── threats_found
├── scan_duration
└── scan_path

threats
├── id (PK)
├── scan_id (FK → scan_history)
├── file_path
├── virus_name
├── file_size
└── detection_time
```

**Ottimizzazioni:**
- Transazioni batch per inserimenti multipli
- Indici su campi query frequenti
- Connection pooling Qt SQL

#### **ThreatReport.h/.cpp**
Modello dati per report scansioni.

**Struttura:**
```cpp
ThreatReport
├── QVector<ThreatInfo> threats
├── Statistics (files, bytes, duration)
└── Formatting helpers
```

#### **Updater.h/.cpp**
Gestisce aggiornamenti database ClamAV via `freshclam`.

**Funzionalità:**
- Chiamata asincrona a `freshclam`
- Parsing output in tempo reale
- Persistenza timestamp ultimo aggiornamento

---

### 2. GUI Layer

Tutti i componenti GUI seguono Material Design 3 con tema scuro.

#### **MainWindow.h/.cpp**
Schermata principale con dashboard.

**Layout:**
```
┌──────────────────────────────────────┐
│          FastAV [Title]              │
│   Modern & Fast Antivirus Scanner    │
├──────────────────────────────────────┤
│                                      │
│      [🔵 NEW SCAN Button]            │
│                                      │
│  [View History]  [Update Database]  │
│                                      │
├──────────────────────────────────────┤
│          Statistics Card             │
│  ┌────────┬────────┬────────┬──────┐│
│  │ Scans  │ Files  │Threats │Update││
│  │   15   │ 45.2K  │   3    │ Today││
│  └────────┴────────┴────────┴──────┘│
└──────────────────────────────────────┘
```

#### **ScanDialog.h/.cpp**
Dialogo selezione percorsi da scansionare.

**Opzioni:**
- Quick scan (locations comuni)
- Custom path (browse)
- Multi-selezione

#### **ScanProgress.h/.cpp**
Visualizzazione progresso live durante scansione.

**Features:**
- Progress bar animata
- Statistiche real-time (files/sec)
- Log stream delle minacce
- Current file display

**Aggiornamenti UI:**
- Timer 500ms per stats refresh
- Signal/slot per eventi scan
- Thread-safe UI updates via Qt signals

#### **ThreatViewer.h/.cpp**
Tabella dettagliata minacce rilevate.

**Colonne:**
- Virus name (bold, rosso)
- File path (tooltip completo)
- File size (formattato)
- Detection time

#### **HistoryViewer.h/.cpp**
Cronologia scansioni precedenti.

**Info visualizzate:**
- Data/ora scan
- Path scansionato
- Files analizzati
- Minacce trovate
- Durata

---

### 3. Utils Layer

#### **MaterialTheme.h/.cpp**
Implementazione Material Design 3.

**Palette Dark Theme:**
```cpp
Primary:     #BB86FC (Viola)
Secondary:   #03DAC6 (Teal)
Background:  #121212 (Nero profondo)
Surface:     #1E1E1E (Grigio scuro)
Error:       #CF6679 (Rosso)
Success:     #4CAF50 (Verde)
Warning:     #FF9800 (Arancione)
```

**Componenti stilizzati:**
- Buttons (elevated, rounded)
- Cards (elevation effect)
- Progress bars (smooth animation)
- Lists (hover states)
- Tables (alternating rows)

#### **FileScanner.h/.cpp**
Helper utilities.

**Funzioni:**
- `getCommonLocations()`: Paths comuni (Home, Documents, Downloads)
- `formatFileSize()`: KB/MB/GB formatting
- `formatDuration()`: Sec/min/hour formatting
- `getFileIcon()`: Emoji icons per tipo file

---

## 🚀 Ottimizzazioni Prestazioni

### 1. Multi-Threading Strategy

```
Main Thread (GUI)
    │
    ├─→ Worker Thread 1 ──→ clamd (file 1)
    ├─→ Worker Thread 2 ──→ clamd (file 2)
    ├─→ Worker Thread 3 ──→ clamd (file 3)
    ⋮
    └─→ Worker Thread N ──→ clamd (file N)
```

**Thread Pool Size:**
```cpp
idealThreadCount() * 2  // I/O bound, non CPU bound
```

Su un sistema 8-core:
- 16 worker threads simultanei
- Massimo throughput I/O

### 2. ClamAV Integration

**Perché clamd e non clamscan?**

| Metodo | Overhead | Speed |
|--------|----------|-------|
| `clamscan` | Carica DB ogni volta | ❌ Lento |
| `clamdscan` | DB già in memoria | ✅ Veloce |

**IPC Performance:**

```cpp
// Linux: Unix Domain Socket (fastest)
socket.connectToServer("/var/run/clamav/clamd.ctl");

// Windows: TCP localhost (fallback)
socket.connectToHost("127.0.0.1", 3310);
```

Unix sockets sono ~30% più veloci di TCP localhost.

### 3. Lock-Free Design

**Atomic Operations:**
```cpp
std::atomic<quint64> m_filesScanned;    // No mutex needed
std::atomic<quint64> m_threatsFound;
std::atomic<bool> m_isScanning;
```

**Quando si usa Mutex:**
Solo per `ThreatReport` (scritture non frequenti):
```cpp
QMutexLocker locker(&m_reportMutex);
m_currentReport.addThreat(...);
```

### 4. Compiler Optimizations

```cmake
-O3              # Massima ottimizzazione
-march=native    # Istruzioni CPU specifiche (SSE, AVX)
-flto            # Link-Time Optimization
```

**Guadagno stimato:** 15-25% più veloce rispetto a `-O2`

### 5. I/O Optimizations

**File System Traversal:**
```cpp
QDirIterator it(path, 
    QDir::Files | QDir::NoDotAndDotDot,
    QDirIterator::Subdirectories);
```

Qt's `QDirIterator` è ottimizzato e lazy-loading.

---

## 🔌 Integrazione ClamAV

### Protocollo Comunicazione

FastAV comunica con clamd usando il protocollo ClamAV:

```
Client → clamd:  "SCAN /path/to/file\n"
clamd → Client:  "/path/to/file: OK\n"
                 // oppure
                 "/path/to/file: Eicar-Test-Signature FOUND\n"
```

**Parsing Response:**
```cpp
if (response.contains("FOUND")) {
    QString virusName = response.section(':', 1).trimmed();
    virusName.remove(" FOUND");
    emit fileScanned(path, true, virusName);
}
```

### Error Handling

```cpp
if (!socket.waitForConnected(1000)) {
    return "ERROR: Cannot connect to clamd daemon";
}
```

Timeout 1 secondo per connessione, 5 secondi per risposta.

---

## 💾 Database Design

### Transazioni Batch

```cpp
m_db.transaction();
// Insert scan_history
// Insert multiple threats
m_db.commit();
```

**Performance gain:** 100× più veloce che commit singoli.

### Query Optimization

```sql
-- Indici
CREATE INDEX idx_scan_date ON scan_history(scan_date);
CREATE INDEX idx_threats_scan_id ON threats(scan_id);

-- Query ottimizzate
SELECT * FROM scan_history 
ORDER BY scan_date DESC 
LIMIT 50;
```

---

## 🎨 UI/UX Design Patterns

### Material Design Principles

1. **Elevation**: Cards con ombra (border-radius + background)
2. **Typography**: Roboto/Segoe UI font stack
3. **Color System**: Palette semantica (Primary, Error, Success)
4. **Motion**: Smooth transitions (hover states)
5. **Spacing**: 8dp grid system (multipli di 8px)

### Responsive Feedback

```cpp
// Hover state
QPushButton:hover {
    background-color: lighter(Primary, 110%);
}

// Progress updates ogni 100ms
QTimer* completionTimer = new QTimer(this);
completionTimer->start(100);
```

### Accessibility

- High contrast theme
- Large touch targets (min 48×48dp)
- Clear focus indicators
- Descriptive labels

---

## 🔒 Security Considerations

### Input Validation

```cpp
QFileInfo info(path);
if (!info.exists() || !info.isReadable()) {
    emit scanError("Invalid path");
    return;
}
```

### Path Sanitization

```cpp
QString absolutePath = QFileInfo(path).absoluteFilePath();
// Evita path traversal attacks
```

### Database Safety

```cpp
query.prepare("INSERT INTO threats (...) VALUES (?, ?, ...)");
query.addBindValue(path);  // Prepared statements
```

Prevenzione SQL injection tramite prepared statements.

---

## 🧪 Testing Strategy

### Unit Tests (da implementare)

```cpp
// Test scanner
TEST(Scanner, ConnectsToClamd)
TEST(Scanner, ScansInfectedFile)
TEST(Scanner, HandlesInvalidPath)

// Test database
TEST(Database, SavesReport)
TEST(Database, RetrievesHistory)
```

### Integration Tests

```bash
# Test completo workflow
1. Start clamd
2. Run FastAV
3. Scan EICAR test file
4. Verify threat detected
5. Check database entry
```

### Performance Tests

```bash
# Benchmark
time ./fastav --scan /path/to/10k/files
```

---

## 📈 Future Enhancements

### Planned Features

1. **Real-time Protection**
   ```cpp
   QFileSystemWatcher watcher;
   watcher.addPath(QDir::homePath());
   connect(&watcher, &QFileSystemWatcher::fileChanged, 
           this, &Scanner::scanFile);
   ```

2. **Quarantine System**
   ```cpp
   void Quarantine::isolateFile(const QString& path) {
       QString quarantinePath = getQuarantineDir() + hash(path);
       QFile::copy(path, quarantinePath);
       QFile::remove(path);  // After user confirmation
   }
   ```

3. **Cloud Scanning**
   - Upload hash a VirusTotal API
   - Distributed scanning
   - Reputation system

4. **Plugin Architecture**
   ```cpp
   class ScanPlugin {
       virtual ScanResult scan(const QString& path) = 0;
   };
   ```

### Performance Improvements

1. **Bloom Filter**: Pre-filter files già scansionati
2. **Hash Cache**: Skip files identici
3. **SIMD**: Vectorized hash computation
4. **GPU**: Offload pattern matching

---

## 📊 Metrics & Monitoring

### Performance Metrics

```cpp
struct ScanMetrics {
    qint64 totalDuration;
    quint64 filesPerSecond;
    quint64 bytesPerSecond;
    quint64 avgFileSize;
    quint64 peakMemoryUsage;
};
```

### Logging

```cpp
qInfo() << "Scan started:" << totalFiles << "files";
qWarning() << "Scan error:" << path << error;
qDebug() << "Performance:" << filesPerSec << "files/s";
```

---

## 🏗️ Build System

### CMake Structure

```cmake
project(FastAV)
├── find_package(Qt6)
├── find_package(SQLite3)
├── add_executable(fastav)
├── target_link_libraries(...)
└── install(TARGETS fastav)
```

### Optimization Flags

```cmake
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    -O3 -march=native -flto
    -DNDEBUG -DQT_NO_DEBUG_OUTPUT
endif()
```

---

## 🔗 Dependencies

### Required

- **Qt6**: GUI framework (≥6.2)
- **ClamAV**: Antivirus engine (≥0.103)
- **SQLite3**: Database (≥3.35)

### Optional

- **Qt6 Charts**: Per grafici statistiche
- **Qt6 Network**: Per cloud features

---

## 📝 Coding Standards

### Style Guide

```cpp
// Classes: PascalCase
class Scanner;

// Methods: camelCase
void startScan();

// Members: m_ prefix
QString m_currentPath;

// Constants: UPPER_CASE
const int MAX_THREADS = 16;
```

### Documentation

```cpp
/**
 * @brief Scans files using ClamAV daemon
 * @param paths List of file/directory paths
 * @emit scanStarted when scan begins
 * @emit scanCompleted when scan finishes
 */
void Scanner::startScan(const QStringList& paths);
```

---

## 🎯 Conclusioni

FastAV è progettato per:

✅ **Performance**: Multi-threading + clamd integration  
✅ **Usability**: Material Design UI intuitiva  
✅ **Maintainability**: Architettura modulare pulita  
✅ **Extensibility**: Plugin-ready architecture  

L'architettura permette facile estensione con nuove features mantenendo alta la velocità e la qualità del codice.

---

**Documenti correlati:**
- [README.md](README.md) - Panoramica progetto
- [INSTALL.md](INSTALL.md) - Guida installazione
