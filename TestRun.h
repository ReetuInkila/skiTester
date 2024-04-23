class TestRun {
public:
    TestRun() : ski(1), rounds(0), t1(nullptr), t2(nullptr) {}

    void addRun(int skiNumber, int rounds) {
        this->ski = skiNumber;
        if (t1 != nullptr) {
            delete[] t1;
        }
        if (t2 != nullptr) {
            delete[] t2;
        }
        this->t1 = new float[rounds];
        this->t2 = new float[rounds];
    }

    void addTimes(float t1, float t2) {
        this->t1[rounds] = t1;
        this->t2[rounds] = t2;
        rounds++;
        
    }

    int getSki() const {
        return ski;
    }

    int getLastRun() const {
        return rounds;
    }

    int getNextRun() const {
        return rounds+1;
    }

    float getLastT1() const {
        if (rounds > 0) {
            return t1[rounds - 1];
        } else {
            return 0.0f;
        }
    }

    float getLastT2() const {
        if (rounds > 0) {
            return t2[rounds - 1];
        } else {
            return 0.0f;
        }
    }

    float avgT1() const {
        return avg(t1, rounds);
    }

    float avgT2() const {
      return avg(t2, rounds);
    }

private:
    int ski;
    int rounds;
    float *t1;
    float *t2;

    float avg(float* arr, int size) const{
        float sum = 0.0f;
        for (int i = 0; i < size; ++i) {
            sum += arr[i];
        }
        if (size > 0) {
            return sum / size;
        } else {
            return 0.0f; // return 0 if array is empty to avoid division by zero
        }
    }

};

