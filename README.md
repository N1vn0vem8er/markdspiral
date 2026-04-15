# Markdspiral

Markdspiral is a markdown editor with spellcheck and preview. It replicates Github's markdown look using [Github markdown css](https://github.com/sindresorhus/github-markdown-css) with allows you to preview markdown files before pushing them.


## Technologies

This project was made with:

* **Qt 6**
* **md4c**
* **nuspell**
* **Github markdown css**

## Build and Install

1. **Clone repository:**

```
git clone https://github.com/N1vn0vem8er/markdspiral.git
```

2. **Create a build directory:**

```
mkdir build && cd build
```

3. **Configure and build:**

```
cmake ..
make
```

4. **To install the application:**

```
make install
```

## License

This software is licensed under GNU GENERAL PUBLIC LICENSE version 3. See [LICENSE](LICENSE) for more information.
It uses css from [github markdown css](https://github.com/sindresorhus/github-markdown-css) by Sindre Sorhus (see [THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md) for information about license).
