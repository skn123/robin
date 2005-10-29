""" HTML Text Formatter

Formats text for viewing across text-based terminals.
"""

import string
from htmllib import HTMLParser
from formatter import AbstractFormatter, NullWriter

class HTMLtoTextFormatter(AbstractFormatter):

	def __init__(self):
		AbstractFormatter.__init__(self, NullWriter())
		self.m_raw = [ ]
		self.page_width = 60
		self.cursor = 0

	def add_line_break(self):
		self.m_raw.append("\n")
		self.cursor = 0
	
	def add_literal_data(self, data):
		self.m_raw.append(str(data))

	def add_flowing_data(self, data):
		words = string.split(data)
		for word in words:
			if self.cursor + len(word) > self.page_width:
				self.m_raw.append("\n")
				self.cursor = 0
			elif self.cursor != 0:
				self.cursor = self.cursor + 1
				self.m_raw.append(" ")
				
			self.cursor = self.cursor + len(word)
			self.m_raw.append(word)

	def end_paragraph(self, blanks):
		self.m_raw.append("\n" * blanks)
		self.cursor = 0

	def getText(self):
		return string.join(self.m_raw, "")


def html2text(htmldata):
	# patch htmldata
	htmldata = htmldata.replace("<br/>", "<br>")
	
	fmt = HTMLtoTextFormatter()
	prs = HTMLParser(fmt)
	prs.feed(htmldata)
	return fmt.getText()
