
### Инструкции по сборке и запуску

1. Убедитесь, что у вас установлены необходимые инструменты: `cmake`, `ninja`, `llvm`, и `protobuf`:

   ```sh
   brew install cmake ninja llvm protobuf
   ```

2. Выполните команду `cmake` для настройки проекта:

   ```sh
   cmake -G Ninja \
   -DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++ \
   -DCMAKE_C_COMPILER=/opt/homebrew/opt/llvm/bin/clang \
   -DTINT_BUILD_FUZZERS=ON \
   -DTINT_BUILD_AST_FUZZER=ON \
   -DTINT_BUILD_REGEX_FUZZER=ON \
   -DTINT_BUILD_STRUCTURE_FUZZER=ON \
   -DTINT_STRUCTURE_FUZZER_SANITIZERS=ON \
   -DCMAKE_OSX_ARCHITECTURES=x86_64 \
   -DProtobuf_INCLUDE_DIR=$(brew --prefix protobuf)/include \
   -DProtobuf_LIBRARY=$(brew --prefix protobuf)/lib/libprotobuf.dylib \
   .
   ```

3. Постройте проект с помощью `ninja`:

   ```sh
   ninja tint_structure_fuzzer
   ```

4. Запустите собранный фаззер:

   ```sh
   ./tint_structure_fuzzer
   ```

5. Если вы хотите видеть генерируемый код на WGSL и информацию об ошибках, установите переменную окружения `TINT_STRUCTURE_FUZZER_DEBUG` перед запуском:

   ```sh
   export TINT_STRUCTURE_FUZZER_DEBUG=1
   ./tint_structure_fuzzer
   ```

6. При желании можно указать свой корпус данных для фаззера, передав путь к нему в командной строке при запуске:

   ```sh
   ./tint_structure_fuzzer <путь_к_вашему_корпусу>
   ```


