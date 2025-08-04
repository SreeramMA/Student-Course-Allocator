#include <bits/stdc++.h>
#define loop(i,a,b) for (int i=a;i<b;i++)
using namespace std;

class course;

class student{
    public:
        string ID;
        string name;
        int YoS; //YoS is the year of study
        set <string> completedCourses;
        set <course *> enrolledCourses;

        //Comparison operators definition
        bool operator<(const student b) const{
            return ID < b.ID;
        }
        bool operator>(const student b) const{
            return ID > b.ID;
        }
        bool operator==(const student b) const{
            return ID == b.ID;
        }

        //Constructor Function
        student(string ID, string name, int YoS, set <string> cc)
           : ID(ID), name(name), YoS(YoS), completedCourses(cc){}

        //Default constructor
        student(){
            ID = "";
        }
};

class course{
    public:
        string code;
        string name;
        int credits;
        int capacity;
        set <course *> prerequisites;
        char timeSlot;
        int currenroll;
        set <student *> currstuds;
        queue <student *> waitlist;

        //Constructor function
        course(string code, string name, int creds, int cap, set <course *> prereq, char ts)
            : code(code), name(name), credits(creds), capacity(cap),
              prerequisites(prereq), timeSlot(ts), currenroll(0){    
        }

        //Default constructor
        course(){
            code = "";
        }

        //Comparison operators definition
        bool operator<(const course b) const{
            return code < b.code;
        }
        bool operator>(const course b) const{
            return code > b.code;
        } 
        bool operator==(const course b) const{
            return code == b.code;
        }     
};

//Checks if a course is already there
course * coursePicker(map <string, course> &courseMap, string code);

//Checks if a student is already there
student * studPicker(map <string, student> &studMap, string sid);

void waitlistProcess(course * courseptr);

bool enroller(course * courseptr, student * studptr);

int minPathFinder(course * source, course * destination, set <char> slotstaken);

int main(){
    map <string, student> studMap;
    map <string, course> courseMap;
    //cout << courseMap["hell"].code;
    int q;
    cin >> q;
    loop(i,0,q){
        //cout << 1;
        string s;
        cin >> s;
        //Add student query
        if (s == "add_student"){
            string sid, name;
            int yos, cc;
            cin >> sid >> name >> yos >> cc;

            //list of completed courses. These did not have the checking of whether they exist or not
            //so i am doing it via a set of strings
            set <string> ccl;
            loop(i,0,cc){
                string temp;
                cin >> temp;
                ccl.insert(temp);
            }
            student temp = student(sid, name, yos, ccl);
            studMap[sid] = temp;
        }
        //Add course query
        else if (s == "add_course"){
            //cout << 1;
            string code, name;
            int creds, cap;
            char slot;
            int prereq;
            cin >> code >> name >> creds >> cap >> slot;
            cin >> prereq;
            int flag = 0;
            set <course *> prel;
            loop(i,0,prereq){
                string temp;
                cin >> temp;
                course * ans = coursePicker(courseMap, temp);
                if (ans == nullptr){
                    flag = 1;
                    break;
                }
                prel.insert(ans);
            }
            if (flag) continue;
            else{
                course temp = course(code, name, creds, cap, prel, slot);
                courseMap[code] = temp;
            }
        }
        //Enroll query
        else if (s == "enroll"){
            string sid, code;
            cin >> sid >> code;
            student * studptr = studPicker(studMap, sid);
            //Checks if student exists
            if (studptr == nullptr){
                continue;
            }
            course * courseptr = coursePicker(courseMap, code);
            //Checks if course exists
            if (courseptr == nullptr){
                continue;
            }
            enroller(courseptr, studptr);

            
        }
        //Print query
        else if (s == "print"){
            string code;
            cin >> code;
            course * courseptr = coursePicker(courseMap, code);
            if (courseptr == nullptr){
                cout << "Invalid Course " << code << endl;
            }
            else{
                cout << "Enrolled students in " << code << ":\n";
                for (auto i : courseptr->currstuds){
                    cout << i->ID << " ";
                } 
                cout << endl;
            }
        } 
        //Drop course query
        else if (s == "drop"){
            string sid, code;
            cin >> sid >> code;
            student * studptr = studPicker(studMap, sid);
            course * courseptr = coursePicker(courseMap, code);
            //Checks if student exists
            if (studptr == nullptr){
                continue;
            }
            //Checks if course exists
            if (courseptr == nullptr){
                continue;
            }
            auto j = courseptr->currstuds.find(studptr);
            if (j != courseptr->currstuds.end()){
                courseptr->currstuds.erase(j);
                courseptr->currenroll--;
            }
            waitlistProcess(courseptr);
        }

        //I don't think a cyclic dependency can arise while adding courses.
        //Let's say A requires B as prereq, B requires C and C requires A. This is an example of cyclic dependency.
        //Without loss of generality, let us add A first. This requires B to be already there.
        //But for B to be there, we need C also. But for that, we need A as well.
        //A does not exist as of now, so C also cannot exist. 
        //Therefore such a cyclic dependency cannot exist in add_course query.

        else if (s=="minEligiblePathlen"){
            string source, destination;
            cin >> source >> destination;
            //cout << 1;
            course * sourceptr = coursePicker(courseMap, source);
            course * destptr = coursePicker(courseMap, destination);
            set <char> slotstaken;   
            cout << minPathFinder(sourceptr, destptr, slotstaken) << endl;         
        }
    }

}

course * coursePicker(map <string, course> &courseMap, string code){
    if (courseMap[code].code == ""){
        return nullptr;
    }
    else return &courseMap[code];
}

student * studPicker(map <string, student> &studMap, string sid){
    if (studMap[sid].ID == "") return nullptr;
    else return &studMap[sid];
}

void waitlistProcess(course * courseptr){
    bool flag = false;
    while (!courseptr->waitlist.empty() && flag == false){
        student * studptr = courseptr->waitlist.front();
        courseptr->waitlist.pop();
        flag = enroller(courseptr, studptr);
    }
}

bool enroller(course * courseptr, student * studptr){
    int flag1 = 0;
    //Checks if capacity is exceeded
    if (courseptr->currenroll == courseptr->capacity){
        courseptr->waitlist.push(studptr);
        return false;
    }

    //Checks prereqs
    for(auto i : courseptr->prerequisites){
        if (studptr->completedCourses.find(i->code) == studptr->completedCourses.end()){
            flag1 = 1;
        }
    }

    //Checks for slot clash
    for (auto i : studptr->enrolledCourses){
        //This will take care if the student is already enrolled in this course as well
        if (i->timeSlot == courseptr->timeSlot){
            flag1 = 1;
        }
    }

    if (flag1) return false;
    studptr->enrolledCourses.insert(courseptr);
    courseptr->currstuds.insert(studptr);
    courseptr->currenroll++;
    return true;
}

int minPathFinder(course * source, course * destination, set <char> slotstaken){
    //cout << 1;
    if (*source == *destination){
        return 1;
    }
    if (slotstaken.find(destination->timeSlot) != slotstaken.end()) return -1;
    slotstaken.insert(destination->timeSlot);
    int m = 15;
    int flag = 0;
    //cout << 2 << endl;
    for (auto i : destination->prerequisites){
        
        int x = minPathFinder(source, i, slotstaken);
        
        if (x == -1) continue;
        else {
            m = min(x+1, m);
            flag = 1;
        }
    }
    if (flag) return m;
    return -1;
}