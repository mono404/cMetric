#include<iostream>
#include<sstream>
#include<vector>
#include<getopt.h>
#include<map>
#include<iomanip>


using namespace std;

string read_cmd(string command);
vector<vector<string> > resToVector(string res);
void print_help(char *program_name);
void getNamespacesResource(string namespaceName);

vector<vector<string> > split_str;
int num = 1;
string nFlag = "0", pFlag = "0", aFlag = "0", cFlag = "0";
string namespaceName = "default", podName, containerName;


int main(int argc, char** argv)
{
    static struct option long_options[] = {
        {"namespace", required_argument, 0, 'N'},
        //{"pod", required_argument, 0, 'P'},
        {"all-namespaces", no_argument, 0, 'A'},
        //{"container", required_argument, 0, 'C'},
        {"help", no_argument, 0, 'H'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    int c;

    if (argc <= 2)
    {
        print_help(argv[0]);
        return 0;
    }

    bool isOpt = false;
    while (1)
    {
        c = getopt_long(argc, argv, "AHN:", long_options, &option_index);
        
        if (c == -1)
            break;

        switch (c)
        {
        case 0:
            isOpt = true;
            if (long_options[option_index].flag != 0)
                break;
            break;
        case 'A':
            isOpt = true;
            aFlag = "1";
            break;
        case 'N':
            isOpt = true;
            nFlag = "1";
            namespaceName = optarg;
            break;
        case 'H':
        default:
            print_help(argv[0]);
            return 0;
        }
    }

    vector<string> v;
    while (optind < argc)
        v.push_back(argv[optind++]);

    if (v[0] != "get")
    {
        print_help(argv[0]);
        return 0;
    }
    
    if (v.size() == 2)
    {
        string input = argv[2];
        
        if (v[1] == "nodes" || v[1] == "node")
        {
            string cmd = "kubectl top node";
            string pod_list = read_cmd(cmd);
            cout << pod_list << "\n";
        }
        else if (v[1] == "namespaces" || v[1] == "namespace")
        {
            namespaceName.clear();
            getNamespacesResource(namespaceName);
        }
        else if (v[1] == "pods" || v[1] == "pod")
        {
            string cmd;
            if (aFlag == "1")
                cmd = "kubectl top pods --all-namespaces";
            else
                cmd = "kubectl top pods --namespace=" + namespaceName;
            string pod_list = read_cmd(cmd);
            cout << pod_list << "\n";
        }
    }
    else
    {
        if (v[1] == "node" || v[1] == "nodes")
        {
            string nodeName = v[2];
            string cmd = "kubectl top node " + nodeName;
            string node_list = read_cmd(cmd);
            cout << node_list << "\n";
        }
        else if (v[1] == "namespace" || v[1] == "namespaces")
        {
            getNamespacesResource(v[2]);
        }
        else if (v[1] == "pod" || v[1] == "pods")
        {
            if (aFlag == "1")
            {
                cout << "error: a resource cannot be retrieved by name across all namespaces" << endl;
                return 0;
            }
            podName = v[2];
            string pod_info = read_cmd("kubectl get pods -A -o custom-columns=CONATINER:.status.containerStatuses[0].containerID,GNAME:.metadata.name,NAMESPACE:.metadata.namespace | grep \"" + namespaceName + "\" | grep \"" + podName + "\"");
            if (pod_info == "error")
            {
                cout << "Error from server (NotFound): pods \"" + podName + "\" not found\n";
                return 0;
            }
            split_str = resToVector(pod_info);

            string containerID = split_str[0][0].substr(13);
            string cmd = "kubectl top pod " + podName + " --namespace=" + namespaceName;
            string out = read_cmd(cmd);
            cout << out << "\n";
            cmd = "crictl exec -it " + containerID + " top -bn 1 | grep \"^ \" | awk '{ printf(\"  %-8s %-8s %-8s\\n\", $9, $10, $12); }' | grep -v \"top\" ";
            out = read_cmd(cmd);
            cout << out << "\n";
        }
    }

    return 0;
}

string read_cmd(string command)
{
    FILE* stream = popen(command.c_str(), "r");
    ostringstream output;

    while (!feof(stream) && !ferror(stream))
    {
        char buf[512];
        int bytesRead;

        if (!(bytesRead = fread(buf, 1, 512, stream)))
        {
            //cout << " -- ERROR! it is single process container! -- \n";
            return "error";
        }
        output.write(buf, bytesRead);
    }
    string res = output.str();
    return res;
}

vector<vector<string> > resToVector(string res)
{
    vector<vector<string> > split_str;
    string tmp = "";
    vector<string> tmpv;
    for (int i = 0; i < res.size(); i++)
    {
        if ((res[i] == 32) && tmp.length() == 0)
            continue;

        if (res[i] == 32)
        {
            tmpv.push_back(tmp);
            tmp = "";
            continue;
        }

        if (res[i] == 10)
        {
            tmpv.push_back(tmp);
            tmp = "";
            split_str.push_back(tmpv);
            tmpv.clear();
            continue;
        }

        tmp += res[i];
    }

    return split_str;
}

void getNamespacesResource(string namespaceName)
{
    string getNamespace = read_cmd("kubectl get namespace");
    vector<vector<string> > split_list = resToVector(getNamespace);
    vector<string> namespace_list;
    map<string, pair<int, int> > m;
    int maxNameWidth = 9, maxCpuWidth = 10, maxMemWidth = 13;

    for (int i = 1; i < split_list.size(); i++)
        m[split_list[i][0]] = { 0, 0 };

    string top_info = read_cmd("kubectl top pod --all-namespaces");
    vector<vector<string> > getTop = resToVector(top_info);
    for (int i = 1; i < getTop.size(); i++)
    {
        int pcpu = stoi(getTop[i][2].substr(0, getTop[i][2].size() - 1));
        int pmem = stoi(getTop[i][3].substr(0, getTop[i][3].size() - 2));

        m[getTop[i][0]].first += pcpu;
        m[getTop[i][0]].second += pmem;

        if (maxNameWidth < getTop[i][0].size())
            maxNameWidth = getTop[i][0].size();
        if (maxCpuWidth < getTop[i][2].size())
            maxCpuWidth = getTop[i][2].size();
        if (maxMemWidth < getTop[i][3].size())
            maxMemWidth = getTop[i][3].size();
    }

    const char separator = ' ';

    cout << left << setw(maxNameWidth + 5) << setfill(separator) << "NAMESPACE";
    cout << left << setw(maxCpuWidth + 5) << setfill(separator) << "CPU(cores)";
    cout << left << setw(maxMemWidth + 5) << setfill(separator) << "MEMORY(bytes)";
    cout << endl;

    if (namespaceName.size() > 0)
    {
        for (auto it = m.begin(); it != m.end(); it++)
        {
            if (it->first == namespaceName)
            {
                string pcpu = to_string(it->second.first);
                pcpu += "m";
                string pmem = to_string(it->second.second);
                pmem += "Mi";

                cout << left << setw(maxNameWidth + 5) << setfill(separator) << it->first;
                cout << left << setw(maxCpuWidth + 5) << setfill(separator) << pcpu;
                cout << left << setw(maxMemWidth + 5) << setfill(separator) << pmem;
                cout << endl;
            }
        }
    }
    else
    {
        for (auto it = m.begin(); it != m.end(); it++)
        {
            string pcpu = to_string(it->second.first);
            pcpu += "m";
            string pmem = to_string(it->second.second);
            pmem += "Mi";

            cout << left << setw(maxNameWidth + 5) << setfill(separator) << it->first;
            cout << left << setw(maxCpuWidth + 5) << setfill(separator) << pcpu;
            cout << left << setw(maxMemWidth + 5) << setfill(separator) << pmem;
            cout << endl;
        }
    }

}

void print_help(char* program_name)
{
    cout << "Display Resource (CPU/Memory/Storage) usage of nodes or namespaces or pods.\n\n";
    cout << "Examples:\n";
    cout << "  # Show metrics for all nodes\n";
    cout << "  " << program_name << " get nodes\n\n";
    cout << "  # Show metrics for all namespaces\n";
    cout << "  " << program_name << " get namespaces\n\n";
    cout << "  # Show metrics for all pods in default namespace\n";
    cout << "  " << program_name << " get pods\n\n";
    cout << "  # Show metrics for all processes in the given pod in the default namespace\n";
    cout << "  " << program_name << " get pods [POD_NAME]\n\n";
    cout << "  # Show metrics for all pods in the given namespace\n";
    cout << "  " << program_name << " get pods --namespace=NAMESPACE\n\n";
    cout << "Options:\n";
    cout << "  -A, --all-namespaces=false: If present, list the request object(s) across all namespaces\n";
    cout << "Namespace in current context is ignored even if specified with --namespace.\n";
    cout << "\nUsage:\n";
    cout << "  " << program_name << " get [flags] [NAME] [option] [(-N|--namespace=)NAMESPACE]\n\n";
}
