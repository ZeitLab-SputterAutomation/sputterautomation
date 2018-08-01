## Building
### Visual Studio 2017
1. Download Visual Studio Community 2017
2. Download and install *Qt* (version 5.11 or up)
3. Install extension *Qt Vs Tools*
4. Download and build *Boost* (from https://www.boost.org/, version 1.6.7 or up)
5. Set up the additional include directory for Boost in the property manager
6. Download and build gtest (from https://github.com/google/googletest)
7. Copy the debug (gtestd.lib) and release (gtest.lib) libraries in their respective folders under `deps/googletest/lib/x64/`
8. Open *sputterautomation.sln* and build the application

Steps 1-5 only need to be done once and steps 6-7 whenever googletest is updated to a newer version (the version used can be seen in `deps/googletest/readme.md`).