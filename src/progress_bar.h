#ifndef __PROGRESS_BAR_H__
#define __PROGRESS_BAR_H__


void progress_bar_init(unsigned long ulMaxvalue);
void progress_bar_set_position(unsigned long ulPosition);
void progress_bar_check_timer(void);
void progress_bar_finalize(void);


#endif  /* __PROGRESS_BAR_H__ */
