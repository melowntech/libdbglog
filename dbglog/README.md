libdbglog: Logging library

Dependencis:
* Boost_THREAD
* Boost_SYSTEM

---

Usage:

Logging using C++ stream:

```c++
LOG(level) << "Some number: " << 10 << ".";
```

Logging using Boost.Format:

```c++
LOG(level)("Some number %d.", 10);
```
