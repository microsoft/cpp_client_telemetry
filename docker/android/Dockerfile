FROM larvacea/builder:latest
RUN cd ${}
RUN chmod +x lib/android_build/gradlew docker/android/assemble.sh
VOLUME /artifacts
CMD ["/bin/sh", "-c", "docker/android/assemble.sh"]
