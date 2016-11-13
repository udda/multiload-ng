# Contributing to Multiload-ng

Thank you for taking the time to contribute to Multiload-ng!

The following is a set of guidelines for contributing to [Multiload-ng](https://github.com/udda/multiload-ng).
These are just guidelines, not rules, use your best judgment and feel free to propose changes to this document in a pull request.



## Contents

- [How Can I Contribute?](#how-can-i-contribute)
	- [Reporting Bugs](#reporting-bugs)
	- [Suggesting Enhancements](#suggesting-enhancements)
	- [Code Contributions](#code-contributions)
	- [Pull Requests](#pull-requests)
	- [Translations](#translations)
	- [Artwork](#artwork)
- [Styleguides](#styleguides)
	- [Git Commit Messages](#git-commit-messages)
	- [C Styleguide](#c-styleguide)
	- [Translations Styleguide](#translations-styleguide)



## How Can I Contribute?

You can contribute to Multiload-ng in many ways.


### Reporting Bugs

This section guides you through reporting a bug in Multiload-ng. Following these guidelines helps maintainers to speed up bug fixing process.

Before creating bug reports, please check [this list](#before-submitting-a-bug-report) as you might find out that you don't need to create one. When you are creating a bug report, please [include as many details as possible](#how-do-i-submit-a-good-bug-report). If you'd like, you can use [this template](#template-for-submitting-bug-reports) to structure the information.


#### Before Submitting A Bug Report

- The bug may have been already solved in development code, and you might just need to wait until next release. Before reporting a bug, build lastest version of Multiload-ng from git source and check whether the bug is still there.
- Read the [Help & Troubleshooting section](https://github.com/udda/multiload-ng/blob/master/README.md#help--troubleshooting) of Multiload-ng README for common issues and how to solve them
- Check in the list of [open issues](https://github.com/udda/multiload-ng/issues?q=is%3Aissue+is%3Aopen) to see if the issue you are describing has already been reported. If it has, add comment to the existing issue instead of opening a new one.



#### How To Submit A (Good) Bug Report?

Bug reports are tracked as [GitHub issues](https://guides.github.com/features/issues/). Create an issue on Multiload-ng repository and provide the following information.

Explain the problem and include additional details to help maintainers reproduce the problem:

- **Use a clear and descriptive title** for the issue to identify the problem.
- **Describe the exact steps which reproduce the problem** in as many details as possible.
- **Provide specific examples to demonstrate the steps**.
- **Describe the behavior you observed after following the steps** and point out what exactly is the problem with that behavior.
- **Explain which behavior you expected to see instead and why.**
- **Include screenshots and animated GIFs** which show you following the described steps and clearly demonstrate the problem. You can use [this tool](https://github.com/colinkeenan/silentcast) or [this tool](https://github.com/GNOME/byzanz) to record GIFs on Linux.
- **You can use [Markdown](https://daringfireball.net/projects/markdown)**.
- **If the problem wasn't triggered by a specific action**, describe what you were doing before the problem happened and share more information using the guidelines below.

Provide more context by answering these questions:

- **Did the problem start happening recently** (e.g. after updating to a new version of Multiload-ng) or was this always a problem?
- If the problem started happening recently, **can you reproduce the problem in an older version of Multiload-ng?** What's the most recent version in which the problem doesn't happen? You can download older versions of Multiload-ng from [the releases page](https://github.com/udda/multiload-ng/releases).
- **Can you reliably reproduce the issue?** If not, provide details about how often the problem happens and under which conditions it normally happens.

Include details about your configuration and environment:

- **Which version of Multiload-ng are you using**?
- **Are you using Multiload-ng inside a panel**? If so, provide panel name and version.
- **Are you using a native plugin**? (e.g. XFCE, MATE or LXDE panel plugin) Or is it a generic one? (that is, not tied to a particular panel, like AppIndicator, System Tray, Standalone)
- **What's the name and version of the OS you're using**?
- **What is the architecture of your CPU**? (32/64 bit - output of `uname -a` can be useful)
- **Are you running Multiload-ng in a virtual machine**? If so, which VM software are you using and which operating systems and versions are used for the host and the guest?
- **Everything else you think it's relevant**. Often, it is.

For some specific categories of bugs, there is some data that will help developers to better understand the problem. This can be a file or the output of a command
You should include these files in your bug report, either attaching them to the issue or linking from a pastebin. Here is the list:

Issue type			| Useful data
:---------			| :----------
Build error			| contents of `config.log` (located in your build directory)
Battery graph		| output of `ls -laR /sys/class/power_supply /sys/class/power_supply/*/`
Temperature graph	| output of `ls -laR /sys/class/hwmon /sys/class/thermal /sys/class/hwmon/*/ /sys/class/thermal/*/`



#### Template For Submitting Bug Reports

	[Short description of problem here]

	**Reproduction Steps:**

	1. [First Step]
	2. [Second Step]
	3. [Other Steps...]


	**Expected behavior:**

	[Describe expected behavior here]


	**Observed behavior:**

	[Describe observed behavior here]


	**Screenshots and GIFs**

	![Screenshots and GIFs which follow reproduction steps to demonstrate the problem](url)


	**Multiload-ng version:** [Enter Multiload-ng version here]
	**OS and version:** [Enter OS name and version here]


	**Additional information:**

	* Problem can be reproduced in safe mode: [Yes/No]
	* Problem started happening recently, didn't happen in an older version of Multiload-ng: [Yes/No]
	* Problem can be reliably reproduced, doesn't happen randomly: [Yes/No]
	* Problem happens unser specific conditions?: [Yes/No]




### Suggesting Enhancements

This section guides you through submitting an enhancement suggestion for Multiload-ng, including completely new features and minor improvements to existing functionality. Following these guidelines helps maintainers and the community understand your suggestion.

Before creating enhancement suggestions, please check [this list](#before-submitting-an-enhancement-suggestion) as you might find out that you don't need to create one. When you are creating an enhancement suggestion, please [include as many details as possible](#how-do-i-submit-a-good-enhancement-suggestion). If you'd like, you can use [this template](#template-for-submitting-enhancement-suggestions) to structure the information.

#### Before Submitting An Enhancement Suggestion

- The enhancement may be already available in development code, and you might just need to wait until next release. Before suggesting an enhancement, build lastest version of Multiload-ng from git source and check whether the enhancement isn't already there.
- Check in the list of [open issues](https://github.com/udda/multiload-ng/issues?q=is%3Aissue+is%3Aopen) to see if the enhancement has already been suggested. If it has, add comment to the existing issue instead of opening a new one.



#### How Do I Submit A (Good) Enhancement Suggestion?

Enhancement suggestions are tracked as [GitHub issues](https://guides.github.com/features/issues/). Create an issue on that repository and provide the following information.

Explain the problem and include additional details to help maintainers reproduce the problem:

- **Use a clear and descriptive title** for the issue to identify the suggestion.
- **Provide a step-by-step description of the suggested enhancement** in as many details as possible.
- **Describe the current behavior** and **explain which behavior you expected to see instead** and why.
- **Include screenshots and animated GIFs** which help you demonstrate the steps or point out the part of Multiload-ng which the suggestion is related to. You can use [this tool](https://github.com/colinkeenan/silentcast) or [this tool](https://github.com/GNOME/byzanz) to record GIFs on Linux.
- **You can use [Markdown](https://daringfireball.net/projects/markdown)**.
- **Explain why this enhancement would be useful** to most users.
- **List some other system monitors or applications where this enhancement exists** (if any).

Include details about your configuration and environment:

- **Which version of Multiload-ng are you using**?
- **What's the name and version of the OS you're using**?
- **Everything else you think it's relevant**. Often, it is.



#### Template For Submitting Enhancement Suggestions

		[Short description of suggestion]

		**Steps which explain the enhancement**

		1. [First Step]
		2. [Second Step]
		3. [Other Steps...]

		**Current and suggested behavior**

		[Describe current and suggested behavior here]

		**Why would the enhancement be useful to most users**

		[Explain why the enhancement would be useful to most users]

		[List some other text editors or applications where this enhancement exists]

		**Screenshots and GIFs**

		![Screenshots and GIFs which demonstrate the steps or part of Atom the enhancement suggestion is related to](url)

		**Multiload-ng Version:** [Enter Multiload-ng version here]
		**OS and Version:** [Enter OS name and version here]



### Code Contributions

You can contribute to Multiload-ng with your coding skills, by closing an existing open issue.

Begin by searching [all open issues with 'help-wanted' label](https://github.com/udda/multiload-ng/issues?q=is%3Aissue+is%3Aopen+label%3A%22help+wanted%22).
Those issues are waiting for you to fix them!

You can also help a lot by just adding a useful comment to an open issue, to give code snippets, hints, usecases, ...).



### Pull Requests

- Include screenshots and animated GIFs in your pull request whenever possible.
- Follow the [C](#c-styleguide) and [Translations](#translations-styleguide) styleguides.
- You can use [Markdown](https://daringfireball.net/projects/markdown).




## Styleguides

### Git Commit Messages

- Use the present tense ("Add feature" not "Added feature")
- Use the imperative mood ("Change filter to..." not "Changes filter to...")
- Limit the first line to 72 characters or less
- Reference issues and pull requests liberally

### C Styleguide

- Place function name and arguments in the same line. Place its return type in a separate line.
- Put opening/closing brackets of functions in separate lines.
- Indent properly, using only tabs. Tab size must be 4 spaces.
- `#include <config.h>` must be the first line in all .c files (excluding comments). Header (.h) files must not include `config.h`.
- Respect the following order in every .c/.h file. Every item must be separated from others by at least one whitespace.
	- `#include <config.h>`
	- `#include` system header files (those with angular brackets), in alphabetical order
	- `#include` local header files (those with double quotes), in alphabetical order. The only exception is that, in .c files, the corresponding header must come first (e.g. in "colors.c", the line `#include "colors.h"` must come before other local includes).
	- `#define`s and macros
	- `typedef`s and structures/unions
	- function declarations/implementations
- Header files must have an include guard:  
<pre>\#ifndef __HEADER_NAME_H
\#define __HEADER_NAME_H
(file contents)
\#endif /* __HEADER_NAME_H */
(EOF)</pre>
- Use spaces after commas.
- Use parentheses if it improves code clarity.
- Use lowercase names for functions and variables, and uppercase names for defines and macros.
- Do not add an explicit `return` at the end of `void` functions.
- Do not add `(void)` argument to function without arguments.
- Use Glib types (`gint`, `gdouble`, `gchar`, ...) when possible.
- Always use Glib printf format specifiers (like `G_GUINT64_FORMAT`) when using Glib types.
- Use `g_new0` when allocating, to avoid buffer overflows and other subtle bugs.
- Add a space between `for`, `while`, `if` and respective parentheses. Same goes for function names and function calls.
- Assume `gnu90` C dialect when you are making use of advanced C features.
- End files with a newline.

### Translations Styleguide

- Do not use indentation, especially on multiline content. Otherwise, some files (like .desktop files) won't be localized.
- Make sure to fill all metadata located at the beginning of file. This includes your name, language, and so on. Please follow format of other existing .po files.
- Do not sort strings. It is useful to have the same order in all .po files.
