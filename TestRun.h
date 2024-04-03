class TestRun {
public:
    TestRun(int skiNumber, int runNumber) : ski(skiNumber), run(runNumber) {}

    void addTimes(float t1, float t2) {
        this->t1 = t1;
        this->t2 = t2;
    }

    int getSki() const {
        return ski;
    }

    int getRun() const {
        return run;
    }

    float getT1() const {
        return t1;
    }

    float getT2() const {
        return t2;
    }

private:
    int ski;
    int run;
    unsigned long t1;
    unsigned long t2;
};
