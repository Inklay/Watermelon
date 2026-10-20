Get-ChildItem -Recurse -File -Include *.cpp,*.cc,*.cxx |
    Where-Object { $_.FullName -notmatch '\\(build|vcpkg_installed|\.git)\\' } |
    ForEach-Object {
        clang-tidy.exe $_.FullName -p build/clang-tidy --quiet --extra-arg=-Wno-unused-command-line-argument
    }