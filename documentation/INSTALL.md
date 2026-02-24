# Guida di Installazione FastAV

Questa guida ti aiuterà a installare e configurare FastAV sul tuo sistema.

## 📦 Installazione su Arch Linux

### 1. Installa le Dipendenze

```bash
# Pacchetti di sistema
sudo pacman -S base-devel cmake qt6-base qt6-tools sqlite clamav

# Se preferisci compilare con GCC (consigliato)
sudo pacman -S gcc
```

### 2. Configura ClamAV

```bash
# Configura il database ClamAV
sudo freshclam

# Abilita e avvia i servizi
sudo systemctl enable clamav-daemon
sudo systemctl enable clamav-freshclam
sudo systemctl start clamav-daemon
sudo systemctl start clamav-freshclam

# Verifica che clamd sia in esecuzione
systemctl status clamav-daemon
```

### 3. Compila FastAV

```bash
# Clona o estrai il progetto
cd fastav

# Usa lo script di build automatico
./build.sh release

# OPPURE manualmente:
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### 4. Installa (Opzionale)

```bash
cd build
sudo make install
```

Ora puoi avviare FastAV con:
```bash
fastav
```

---

## 🐧 Installazione su Ubuntu/Debian

### 1. Installa le Dipendenze

```bash
# Pacchetti di sistema
sudo apt update
sudo apt install -y build-essential cmake git

# Qt6
sudo apt install -y qt6-base-dev qt6-tools-dev libqt6sql6-sqlite

# ClamAV
sudo apt install -y clamav clamav-daemon clamav-freshclam

# SQLite
sudo apt install -y libsqlite3-dev
```

**Nota**: Su Ubuntu < 22.04, Qt6 potrebbe non essere disponibile. In tal caso:

```bash
# Aggiungi PPA per Qt6
sudo add-apt-repository ppa:okirby/qt6-backports
sudo apt update
sudo apt install qt6-base-dev
```

### 2. Configura ClamAV

```bash
# Ferma i servizi se in esecuzione
sudo systemctl stop clamav-freshclam
sudo systemctl stop clamav-daemon

# Aggiorna il database
sudo freshclam

# Avvia i servizi
sudo systemctl start clamav-freshclam
sudo systemctl start clamav-daemon
sudo systemctl enable clamav-daemon
sudo systemctl enable clamav-freshclam

# Verifica
systemctl status clamav-daemon
```

### 3. Compila FastAV

```bash
cd fastav
./build.sh release

# O manualmente
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### 4. Installa

```bash
cd build
sudo make install
```

---

## 🪟 Installazione su Windows

### 1. Installa le Dipendenze

#### Qt6
1. Scarica Qt Online Installer da [qt.io](https://www.qt.io/download-qt-installer)
2. Installa Qt 6.5+ con componenti:
   - Qt 6.x for MSVC 2019 (o 2022)
   - CMake
   - Ninja

#### Visual Studio
1. Scarica [Visual Studio 2022 Community](https://visualstudio.microsoft.com/)
2. Installa con workload "Desktop development with C++"

#### ClamAV
1. Scarica [ClamAV per Windows](https://www.clamav.net/downloads)
2. Estrai in `C:\Program Files\ClamAV`
3. Copia `conf_examples\clamd.conf.sample` in `clamd.conf`
4. Modifica `clamd.conf`:
   ```
   # Decommenta questa riga:
   TCPSocket 3310
   TCPAddr 127.0.0.1
   ```

#### SQLite
SQLite è incluso con Qt, nessuna installazione aggiuntiva necessaria.

### 2. Configura ClamAV

```cmd
# Apri CMD come amministratore
cd "C:\Program Files\ClamAV"

# Aggiorna database
freshclam.exe

# Avvia clamd (in una finestra separata)
clamd.exe
```

Per avviare clamd automaticamente, crea un servizio Windows o aggiungi allo startup.

### 3. Compila FastAV

#### Usando Qt Creator (Più Facile)
1. Apri Qt Creator
2. File → Open File or Project → Seleziona `CMakeLists.txt`
3. Configura il progetto con il kit MSVC
4. Build → Build Project "FastAV"
5. Run

#### Usando Command Line
```cmd
# Apri "Developer Command Prompt for VS 2022"
cd fastav
mkdir build
cd build

# Configura con CMake
cmake -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH=C:\Qt\6.5.0\msvc2019_64 ..

# Compila
cmake --build . --config Release

# Esegui
Release\fastav.exe
```

---

## 🔧 Configurazione Post-Installazione

### Verifica ClamAV

```bash
# Linux
systemctl status clamav-daemon

# Testa connessione
echo "PING" | nc -U /var/run/clamav/clamd.ctl  # Linux
# oppure
echo "PING" | nc 127.0.0.1 3310  # Windows
```

Dovresti ricevere "PONG".

### Aggiorna Database Virus

```bash
# Manualmente
sudo freshclam  # Linux
freshclam.exe   # Windows

# FastAV può anche aggiornare automaticamente
# dall'interfaccia: "Update Database"
```

### Permessi (Linux)

Se hai problemi di permessi con clamd:

```bash
# Aggiungi il tuo utente al gruppo clamav
sudo usermod -a -G clamav $USER

# Riavvia la sessione o usa
newgrp clamav
```

---

## 🐛 Risoluzione Problemi Comuni

### "Cannot connect to clamd daemon"

**Linux:**
```bash
# Verifica che clamd sia in esecuzione
ps aux | grep clamd

# Controlla il socket
ls -la /var/run/clamav/clamd.ctl

# Verifica log
sudo journalctl -u clamav-daemon -n 50
```

**Windows:**
```cmd
# Verifica processo
tasklist | findstr clamd

# Controlla se la porta 3310 è in ascolto
netstat -an | findstr 3310
```

### "Qt platform plugin could not be initialized"

```bash
# Linux - installa plugin Qt
sudo apt install qt6-qpa-plugins  # Ubuntu
sudo pacman -S qt6-base          # Arch

# Imposta variabile ambiente
export QT_QPA_PLATFORM=xcb
```

### Build fails con errori Qt

```bash
# Assicurati che qmake6 sia nel PATH
export PATH=/usr/lib/qt6/bin:$PATH

# O specifica esplicitamente
cmake -DCMAKE_PREFIX_PATH=/usr/lib/qt6 ..
```

### ClamAV database non aggiornato

```bash
# Forza aggiornamento
sudo freshclam --verbose

# Se fallisce, controlla configurazione
sudo nano /etc/clamav/freshclam.conf
```

---

## 📊 Ottimizzazioni Prestazioni

### Aumenta Thread Pool

Modifica in `Scanner.cpp`:
```cpp
// Linea ~63
m_threadPool->setMaxThreadCount(QThread::idealThreadCount() * 4);
// Aumenta il moltiplicatore per più thread
```

### Abilita Ottimizzazioni Compiler

Nel `CMakeLists.txt`:
```cmake
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(fastav PRIVATE 
        -O3           # Massima ottimizzazione
        -march=native # Ottimizza per CPU corrente
        -flto         # Link-time optimization
    )
endif()
```

### Aumenta Memoria clamd

Modifica `/etc/clamav/clamd.conf`:
```
# Aumenta limite scansione
MaxScanSize 500M
MaxFileSize 100M
```

---

## 🎯 Prossimi Passi

Dopo l'installazione:

1. **Aggiorna il database**: Clicca "Update Database" nell'interfaccia
2. **Fai una scansione di prova**: Seleziona una cartella piccola
3. **Controlla le statistiche**: Vedi la cronologia scansioni
4. **Personalizza**: Modifica temi e impostazioni nel codice

---

## 📞 Supporto

Se incontri problemi:

1. Controlla i log del sistema
2. Verifica che tutte le dipendenze siano installate
3. Apri una issue su GitHub con:
   - Sistema operativo e versione
   - Output completo dell'errore
   - Log di ClamAV (`/var/log/clamav/`)

---

**Buona scansione!** 🛡️
