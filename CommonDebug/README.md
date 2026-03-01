# CommonDebug

Macro minime per il logging seriale condizionato da `DEBUG`.

## API

```cpp
DBG_LOG("*TAG:", "message");
DBG_LOGF("*TAG:", "value=%d\n", value);
```

## Note

- Se `DEBUG` non e definito, le macro non producono output.
- Le librerie del progetto usano questo helper per log compatti e uniformi.
