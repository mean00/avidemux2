This is a proposal to improve the Avidemux organization.

# Problems
Avidemux is scattered around a bunch of websites:
- main: https://avidemux.sourceforge.net/ + https://sourceforge.net/projects/avidemux/
- download: https://github.com/mean00/avidemux2/releases + https://sourceforge.net/projects/avidemux/files/latest/download
- code: https://github.com/mean00/avidemux2 + https://github.com/flathub/org.avidemux.Avidemux + https://sourceforge.net/projects/avidemux/files/
- wiki: https://www.avidemux.org/admWiki/doku.php + https://sourceforge.net/p/avidemux/wiki/Home/
- forum: https://www.avidemux.org/admForum/
- mailing list: https://sourceforge.net/p/avidemux/mailman/
- bug tracker: https://sourceforge.net/p/avidemux/_list/tickets
https://avidemux.sourceforge.net/ doesn't even link to all of those (especially bug tracker + code).
Additionally, searching on the Internet doesn't list those websites directly either.

This leads to a situation where people believe that there is no bug tracker at all or that you cannot contribute to the code
(I'm not making this up, see e.g. https://github.com/flathub/org.avidemux.Avidemux/issues/8#issuecomment-611108547).

Even if the discoverability issue is fixed, it would still require a user to create 3-4 different accounts to get involved.
You just cannot expect this to happen.

# Consequences
- Users rather do not create bug reports/find the help they need.
- Developers rather do not contribute.

In total: Avidemux is a worse product/experience than it could be.

# Proposal
Move to GitHub.
- The code is already here.
- Many people own a GitHub account.
- Less maintenance effort.

Steps:
- Create an avidemux organization. A project like this should really not be tied to a user account.
- Give admin permissions for this organization to multiple contributors.
- Replace sourceforge bug tracker with GitHub issues.
- Replace https://avidemux.sourceforge.net/ with GitHub pages. This will allow contributions to the website (and, honestly, it looks really dated).
- Replace wiki with GitHub wiki. Use wiki as read only user guide/FAQ.
- Remove mailing list. Use RSS (GitHub release feed, GitHub pages news section) and/or social media to inform about new versions.
- Optional: Replace forum with GitHub discussions. It would be more accessible (for users which have problems, but also for others who might be willing to help)
  but I can understand if you prefer a separate forum.
