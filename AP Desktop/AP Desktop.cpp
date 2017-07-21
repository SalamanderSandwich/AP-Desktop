// AP Desktop.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <conio.h>
#include <algorithm>
#include <regex>
#include <curl/curl.h>
#include <curl/easy.h>

//abstract better
//comment
//spell check
//make url appear (maybe a setting to hide it) (like a file named debug)
//make title appear every time
//program doesn't need to close for "login error" it should just call the info loop at the start.
//after login "Anime Recommendations, Reviews, Manga and More!"

using namespace std;

CURL *curl;
string currentLoc;

void curlSetup()
{
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");//start cookie jar
	curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);//probably unnecessary, but can't really hurt matters
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);//redirect if needed
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);//verify everything
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);//
	curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");//certificate list get it from here if you're paranoid https://curl.haxx.se/docs/caextract.html
}

//Cleans the description from ap
string cleanHTMLText(string s)
{
	const int ARRAY_LENGTH = 7;
	//squiggles are the html code, translation is the actual text 
	string htmlSquiggles[] = { "&rsquo;","&quot;","&lsquo;","&ldquo;","&rdquo;","&ndash;","&hellip;" };
	string translation[] = { "’","\"","‘","\"","\"","-","..." };
	for (int x = 0; x<ARRAY_LENGTH; x++)//for every element
	{
		size_t codeCheck = 0;
		while ((codeCheck = s.find(htmlSquiggles[x], codeCheck)) != string::npos)//find code in description for every occurance
		{
			s = s.substr(0, codeCheck) + translation[x] + s.substr(codeCheck + htmlSquiggles[x].size(), s.size() - htmlSquiggles[x].size() - codeCheck);//replace code
			codeCheck -= htmlSquiggles[x].size();//move the check back
		}
	}
	return s;
}

void downloadFile(string url, string filepath)//download page to file, this will be replaced later because downloading to files is stupid
{
	CURLcode res;
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	FILE *fileout;
	fileout = fopen(filepath.c_str(), "wb");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fileout);
	res = curl_easy_perform(curl);
	if (res != CURLE_OK)
	{
		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		if (res == CURLE_OPERATION_TIMEDOUT || res == CURLE_COULDNT_RESOLVE_HOST)//because i'm lazy the program fails to connect. The reason it has to end has to do with deleting the login file, idk need to mess with it more.
		{
			cout << "Check your internet connection, ap may be under DDoS protection.\nProgram will end.";
			_getch();
			exit(0);
		}
	}
	fclose(fileout);
}

void downloadFile(string url)//overloading to the default file path
{
	downloadFile(url, "temp.html");
}

string lowerCaser(string s)
{
	for (int x = 0; x<(int)s.size(); x++)
	{
		s[x] = tolower(s[x]);
	}
	return s;
}

string convertInt(int number)
{
	stringstream ss;//create a stringstream
	ss << number;//add number to the stream
	return ss.str();//return a string with the contents of the stream
}

void doSetup()//i'm a very bad programer
{
	string userName, passWord, command, sessionID;
	bool leave = false;
	for (; leave == false;)
	{
		ofstream addLogin("ap.login");
		cout << "Please input your Username: ";
		cin >> userName;
		cout << "Please input your Password: ";
		cin >> passWord;
		addLogin << userName << endl << passWord;
		cout << "Logging in... ";
		string postData = "username=" + userName + "&password=" + passWord;
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)postData.size());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
		curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "ap.cookie");
		downloadFile("https://www.anime-planet.com/login.php");
		curl_easy_cleanup(curl);
		curlSetup();
		curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "ap.cookie");
		fstream temp("temp.html");
		string tempLine;
		smatch m;
		regex expression("submitFauxForm.+value=\"");
		while (getline(temp, tempLine))
		{
			while (regex_search(tempLine, m, expression))
			{
				size_t last = m.suffix().str().find("\"");
				sessionID = m.suffix().str().substr(0, last);
				leave = true;
				break;
			}
		}
		if (leave == false)
		{
			cout << "Error try again" << endl;
		}
		temp.close();
		addLogin.close();
	}
	cin.sync();//clears everything out
}

int main(int argc, char* argv[])
{
	currentLoc = argv[0];
	size_t last = currentLoc.find_last_of("\\");
	currentLoc = currentLoc.substr(0, last + 1);
	//curl_global_init(CURL_GLOBAL_DEFAULT);
	//curl = curl_easy_init();
	//curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");//start cookie jar
	//curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);//probably unnecessary, but can't really hurt matters
	//curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);//redirect if needed
	//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
	//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);
	//curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
	doSetup();
	string command;
	string sessionID;
	fstream login("ap.login");
	if (!login)//if file doesn't exist
	{
		login.close();//close the file
		doSetup();//start setup
	}
	else
	{
		string userName, passWord;
		bool leave = false;
		cout << "Logging in... ";
		fstream cookieCheck("ap.cookie");
		if (cookieCheck)
		{
			cout << "checking cookie... ";
			curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "ap.cookie");
			downloadFile("https://www.anime-planet.com/contact");
			fstream temp("temp.html");
			string tempLine;
			smatch m;
			regex expression("submitFauxForm.+value=\"");
			while (getline(temp, tempLine))
			{
				while (regex_search(tempLine, m, expression))
				{
					size_t last = m.suffix().str().find("\"");
					sessionID = m.suffix().str().substr(0, last);
					leave = true;
					break;
				}
			}
			temp.close();
		}
		cookieCheck.close();
		if (leave == false)
		{
			getline(login, userName);
			getline(login, passWord);
			cout << "cookie died, using login info... ";
			string postData = "username=" + userName + "&password=" + passWord;
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)postData.size());
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
			curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "ap.cookie");
			downloadFile("https://www.anime-planet.com/login.php");
			curl_easy_cleanup(curl);
			curlSetup();
			curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "ap.cookie");
			fstream temp("temp.html");
			string tempLine;
			smatch m;
			regex expression("submitFauxForm.+value=\"");
			while (getline(temp, tempLine))
			{
				while (regex_search(tempLine, m, expression))
				{
					size_t last = m.suffix().str().find("\"");
					sessionID = m.suffix().str().substr(0, last);
					leave = true;
					break;
				}
			}
			if (leave == false)
			{
				cout << "Login error, please enter your login information." << endl;
				temp.close();
				login.close();
				doSetup();
			}
			else
			{
				//cout<<"used login info... ";
			}
			temp.close();
			login.close();
		}
		else
		{
			cout << "cookie good... ";
		}
	}
	cout << "done" << endl;
	string fileLoc = "temp.html";
	for (;;)
	{
		string apUrl;
		string line;
		cout << "> ";
		char input[100];
		cin.getline(input, sizeof(input));
		line = input;
		size_t findUser = line.find("!user ");
		if (findUser != string::npos)
		{
			string user = line.substr(findUser + 6, line.size() - findUser - 6);
			string url = "https://www.anime-planet.com/users/" + user;
			downloadFile("https://www.anime-planet.com/users/" + user);
			ifstream downloadedThing(fileLoc);
			string currentLine;
			bool foundIt = false;
			while (getline(downloadedThing, currentLine) && foundIt == false)
			{
				size_t joined = currentLine.find("Joined");
				if (joined != string::npos)
				{
					foundIt = true;
					url = "start " + url;
					system(url.c_str());
				}
			}
			if (foundIt == false)
			{
				cout << "Sorry no user found" << endl;
			}
			downloadedThing.close();
		}
		else//anime stuffs
		{
			string dataID;
			string bestAnimeTitle;
			bool useDefault = false;
			size_t check = line.find(" -");
			if (check == string::npos)
			{
				useDefault = true;
			}
			string animeTitle = line.substr(0, check);
			string url = "https://www.anime-planet.com/anime/all?name=" + animeTitle;
			size_t spaceCheck = 0;
			for (spaceCheck = url.find(" ", spaceCheck); spaceCheck != string::npos; spaceCheck = url.find(" ", spaceCheck))//find apostrophe
			{
				url = url.substr(0, spaceCheck) + "%20" + url.substr(spaceCheck + 1, url.size() - 1 - spaceCheck);
			}
			downloadFile(url);
			ifstream downloadedThing(fileLoc);
			string currentLine;
			bool foundIt = false;
			while (getline(downloadedThing, currentLine) && foundIt == false)
			{
				size_t lookFor = currentLine.find("content='http");//is the default result
				if (lookFor != string::npos)
				{
					size_t lookFor2 = currentLine.find("'/>", lookFor);
					apUrl = currentLine.substr(lookFor + 9, lookFor2 - lookFor - 9);
					lookFor = currentLine.find("property='og:title'");
					lookFor += 29;
					lookFor2 = currentLine.find("'", lookFor);
					bestAnimeTitle = currentLine.substr(lookFor, lookFor2 - lookFor);
					while (getline(downloadedThing, currentLine) && foundIt == false)
					{
						lookFor = currentLine.find("data-id=");
						if (lookFor != string::npos)
						{
							lookFor += 9;
							size_t lookFor2 = currentLine.find('"', lookFor);
							dataID = currentLine.substr(lookFor, lookFor2 - lookFor);
							foundIt = true;
						}
					}
				}
				size_t searchLook = currentLine.find("cardDeck");
				if (searchLook != string::npos)
				{
					//cout<<"idaso"<<endl;
					string currentDataID, currentTitle, currentApUrl;
					string bestDataID = "POOP";
					size_t currentScore, bestScore;
					bestScore = 100000000;
					while (getline(downloadedThing, currentLine))
					{
						lookFor = currentLine.find("tooltip anime");
						if (lookFor != string::npos)
						{
							lookFor += 13;
							size_t lookFor2 = currentLine.find('"', lookFor);
							currentDataID = currentLine.substr(lookFor, lookFor2 - lookFor);
							//cout<<currentDataID<<endl;
							lookFor = currentLine.find("href=");
							lookFor += 6;
							lookFor2 = currentLine.find('"', lookFor);
							currentApUrl = currentLine.substr(lookFor, lookFor2 - lookFor);
						}
						else
						{
							lookFor = currentLine.find("<h4>");
							if (lookFor != string::npos)
							{
								lookFor += 4;
								size_t lookFor2 = currentLine.find("</h4", lookFor);
								currentTitle = currentLine.substr(lookFor, lookFor2 - lookFor);
								currentScore = currentTitle.size();
								currentTitle = lowerCaser(currentTitle);
								animeTitle = lowerCaser(animeTitle);
								//cout<<animeTitle.size();
								//cout<<currentTitle<<endl;
								size_t check = currentTitle.find(animeTitle);
								//cout<<"checking"<<endl;
								if (check != string::npos)
								{
									//cout<<"found title"<<endl;
									currentScore -= animeTitle.size();
									//cout<<currentScore<<endl;
									if (check == 0)
									{
										//cout<<"starts with"<<endl;
										currentScore /= 2;
									}
									if (currentScore<bestScore)
									{
										//cout<<"winner"<<endl;
										bestScore = currentScore;
										bestDataID = currentDataID;
										apUrl = "www.anime-planet.com" + currentApUrl;
										bestAnimeTitle = currentLine.substr(lookFor, lookFor2 - lookFor);
									}
								}
							}
						}
					}
					dataID = bestDataID;
					if (dataID != "POOP")
					{
						foundIt = true;
					}
				}
			}
			if (foundIt == false)
			{
				cout << "Sorry couldn't find anything." << endl;
			}
			else
			{
				check = line.find(" -?");//description, status, rating
				if (check != string::npos || useDefault == true)
				{
					//datePublished or season1
					downloadFile(apUrl);
					fstream file("temp.html");
					string line;
					string last;
					bool foundDisplay = false;
					bool foundRating = false;
					while (getline(file, line))
					{
						check = line.find("<title>");
						if (check != string::npos)
						{
							size_t check2 = line.find(" |", check);
							cout << line.substr(check + 7, check2 - check - 7) << endl;
						}
						check = line.find("itemprop='description'");
						if (check != string::npos)
						{
							size_t check2 = line.find("</p>", check);
							string temp = cleanHTMLText(line.substr(26 + check, check2 - check - 26));//to avoid touching the full  a temp is made to be messed with
							string finalS = "";//create temp string
							for (;;)
							{
								string searchThing = temp.substr(0, 81);//make a segment of string to make searching easier
								size_t spacePos = searchThing.find_last_of(" ", 81);//fins last space to make
								if (spacePos != string::npos)//is there a space?
								{
									if (spacePos == 80)//checks to see if it's at the end, because a new line isn't needed
									{
										finalS += temp.substr(0, spacePos);//copy text to new string
										temp = temp.substr(spacePos + 1, temp.size() - spacePos + 1);//cut text out of the string
									}
									else
									{
										finalS += temp.substr(0, spacePos);//copy text to new string
										finalS += '\n';//make new line
										temp = temp.substr(spacePos + 1, temp.size() - spacePos + 1);//cut text out of line
									}
								}
								else//is line a single word?
								{
									finalS += temp.substr(0, 79);//get text for new string
									finalS += "-";//add - to break line
									temp = temp.substr(79, temp.size() - 79);//cut text out of line
								}
								if (temp.size()<81)//does remaining text all fit on one line?
								{
									finalS += temp;//move all text to new string
									break;//exit loop
								}
							}
							cout << finalS << endl;
						}
						check = line.find("display status");
						if (check != string::npos&&foundDisplay == false)
						{
							foundDisplay = true;
							char status = line[check + 14];
							switch (status - '0')
							{
							case 0:
								cout << "Not Watching" << endl;
								break;
							case 1:
								cout << "Watched" << endl;
								break;
							case 2:
							{
								cout << "Watching ";
								string totalEpisodeCount;
								while (getline(file, line))
								{
									check = line.find("select data-eps");
									if (check != string::npos)
									{
										check += 17;
										size_t check2 = line.find('"', check);
										totalEpisodeCount = line.substr(check, check2 - check);
									}
									check = line.find("selected=");
									if (check != string::npos)
									{
										check = line.find(">", check);
										check++;
										size_t check2 = line.find("<", check);
										cout << " " << line.substr(check, check2 - check) << "/" << totalEpisodeCount;
										break;
									}
								}
								cout << endl;
								break;
							}
							case 3:
							{
								cout << "Dropped";
								string totalEpisodeCount;
								while (getline(file, line))
								{
									check = line.find("select data-eps");
									if (check != string::npos)
									{
										check += 17;
										size_t check2 = line.find('"', check);
										totalEpisodeCount = line.substr(check, check2 - check);
									}
									check = line.find("selected=");
									if (check != string::npos)
									{
										check = line.find(">", check);
										check++;
										size_t check2 = line.find("<", check);
										cout << " " << line.substr(check, check2 - check) << "/" << totalEpisodeCount;
										break;
									}
								}
								cout << endl;
								break;
							}
							case 4:
								cout << "Want to Watch" << endl;
								break;
							case 5:
							{
								cout << "Stalled";
								string totalEpisodeCount;
								while (getline(file, line))
								{
									check = line.find("select data-eps");
									if (check != string::npos)
									{
										check += 17;
										size_t check2 = line.find('"', check);
										totalEpisodeCount = line.substr(check, check2 - check);
									}
									check = line.find("selected=");
									if (check != string::npos)
									{
										check = line.find(">", check);
										check++;
										size_t check2 = line.find("<", check);
										cout << " " << line.substr(check, check2 - check) << "/" << totalEpisodeCount;
										break;
									}
								}
								cout << endl;
								break;
							}
							case 6:
								cout << "Won't Watch" << endl;
								break;
							}
						}
						check = line.find("starrating");
						if (check != string::npos&&foundRating == false)
						{
							foundRating = true;
							getline(file, line);
							check = line.find("title");
							check += 7;
							size_t check2 = line.find('"', check);
							cout << line.substr(check, check2 - check) << endl;
						}
						check = line.find("aggregateRating");
						if (check != string::npos)
						{
							check = line.find("title");
							check += 7;
							size_t check2 = line.find("'", check);
							last = line.substr(check, check2 - check);
						}
						check = line.find("season1");
						if (check != string::npos)
						{
							size_t checkEnd = line.find("</a>");
							size_t checkFirst = line.find_last_of(">", checkEnd);
							cout << "Aired: " + line.substr(checkFirst + 1, checkEnd - checkFirst - 1) << endl;
						}
					}
					cout << last << endl;
				}
				else
				{
					cout << "Title found: " << bestAnimeTitle << endl;
				}
				size_t checkW2W = line.find(" -w2w");//want to watch
				if (checkW2W != string::npos)
				{
					downloadFile("https://www.anime-planet.com/api/list/status/anime/" + dataID + "/4/" + sessionID);
				}
				check = line.find(" -w");//watching
				if (check != string::npos&&checkW2W == string::npos)
				{
					downloadFile("https://www.anime-planet.com/api/list/status/anime/" + dataID + "/2/" + sessionID);
				}
				check = line.find(" -f");//finished
				if (check != string::npos)
				{
					downloadFile("https://www.anime-planet.com/api/list/status/anime/" + dataID + "/1/" + sessionID);
				}
				check = line.find(" -d");//dropped
				if (check != string::npos)
				{
					downloadFile("https://www.anime-planet.com/api/list/status/anime/" + dataID + "/3/" + sessionID);
				}
				check = line.find(" -e ");//episode
				if (check != string::npos)
				{
					check += 4;
					size_t check2 = line.find(" ", check);
					if (check2 == string::npos)
					{
						check2 = line.size();
					}
					string atEpisode = line.substr(check, check2 - check);
					downloadFile("https://www.anime-planet.com/api/list/update/anime/" + dataID + "/" + atEpisode + "/" + sessionID);
				}
				check = line.find(" -r "); //rate
				if (check != string::npos)
				{
					check += 4;
					size_t check2 = line.find(" ", check);
					if (check2 == string::npos)
					{
						check2 = line.size();
					}
					string ratting = line.substr(check, check2 - check);
					int rating = stoi(ratting);
					if (rating>5)
					{
						int newRat = (int)(rating / 2);
						ratting = convertInt(newRat);
						if (rating % 2 == 1)
						{
							ratting += ".5";
						}
						cout << "Correcting to " << ratting << "/5" << endl;
					}
					downloadFile("https://www.anime-planet.com/api/list/rate/anime/" + dataID + "/" + ratting + "/" + sessionID);
				}
				check = line.find(" -s");//search
				if (check != string::npos)
				{
					//command="start https://www.anime-planet.com/users/review_edit_review.php?anime_id="+dataID;
					command = "start " + apUrl;
					system(command.c_str());
				}
			}
		}
	}
}

