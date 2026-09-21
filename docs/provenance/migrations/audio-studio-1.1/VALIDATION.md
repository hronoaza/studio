> Historical package-preparation snapshot. For the current repository CI result, see `CI_VALIDATION.md`. The later GitHub Actions run #8 passed the browser/headless suite for commit `d9f1f38f54ea9cd0a8216f805daa9d415edc7b8d`.

# Статус валідації пакета 1.1

| Перевірка | Результат |
|---|---|
| Node.js: 13 тестів конфігурації, ownership і асинхронного життєвого циклу | PASS, 13/13 |
| Синтаксис JS, JSON та наявність локальних ресурсів | Перевірено під час підготовки пакета |
| 22 браузерні тести: OfflineAudioContext та вибрані async-регресії | НЕ ВИКОНАНО |
| Headless UI: HiDPI, вузький viewport, маніфест у підкаталозі | НЕ ВИКОНАНО |
| Прослуховування, Firefox, Safari, мобільні пристрої | НЕ ВИКОНАНО |

Команда `node --test tests/lifecycle.test.mjs` завершилася з кодом 0: 13 passed, 0 failed. Використано контрольовані замінники AudioContext/AudioNode; ці результати не є вимірюваннями DSP.

`node tests/run-headless.mjs` завершився з кодом 1 до запуску тестів: виконуваний файл Chromium відсутній. Спроба завантаження браузера Playwright завершилася тайм-аутами та HTTP 502. Значення піків, DC та реакції компресора не вигадувалися і не записувалися як виміряні.

Після встановлення браузера виконайте `npm run test:audio`. Успішно запущений набір збереже `test-results.json`, у тому числі версію браузера, допуски через результати assert та фактичні виміри. Звіт не є сертифікацією фізичної гучності чи захисту міжсемплових піків.
