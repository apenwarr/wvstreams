#include "wvpam.h"
#include "wvcrash.h"

int main(int argc, char **argv)
{
    wvcrash_setup(argv[0]);
    
    WvString username;
    WvString password;
    
    if (getuid())
    {
	fprintf(stderr, "This test will fail since you are not root\n");
	return -1;
    }
    
    if (argc > 0)
	username = argv[1];
    else
	username = "test";
    
    if (argc > 1)
	password = argv[2];
    else
	password = "foo";
    
    WvPam pam("Really Dumb");
    if (pam.err.isok())
    {
	fprintf(stderr, "\nShould have failed!\n\n");
	return 1;
    }
    
    WvPam pam1("Dumb", WvString::null, username, "flummuxednotapassword");
    if (pam1.err.isok())
    {
	fprintf(stderr, "\nShould have failed!\n\n");
	return 1;
    }
    
    fprintf(stderr, "\nPASSED username and bad password\n\n");
    
    WvPam pam2("Dumber", WvString::null, username, password);
    if (!pam2.err.isok())
    {
	fprintf(stderr, "\nShould have succeeded!\n\n");
	return 1;
    }
    
    fprintf(stderr, "\nPASSED username and good password\n\n");
    
    WvPam pam3("Dumbest", "localhost", username, "notarealpassword");
    if (pam3.err.isok())
    {
	fprintf(stderr, "\nShould have failed!\n\n");
	return 1;
    }   

    fprintf(stderr, "\nPASSED RHOST + username and bad password\n\n");

    WvPam pam4("Most Dumb", "localhost", username, WvString::null);
    if (pam4.err.isok())
    {
	fprintf(stderr, "\nShould have failed!\n\n");
	return 1;
    }
    
    fprintf(stderr, "\nPASSED RHOST + username and no password\n\n");

}
